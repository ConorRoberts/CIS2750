"use strict";

// C library API
const ffi = require("ffi-napi");

// Express App (Routes)
const express = require("express");
const app = express();
const path = require("path");
const fileUpload = require("express-fileupload");
const mysql = require("mysql2/promise");
const _ = require('lodash');

app.use(fileUpload());
app.use(express.static(path.join(__dirname + "/uploads")));
app.use(express.json());

// Minimization
const fs = require("fs");
const JavaScriptObfuscator = require("javascript-obfuscator");
const { arch } = require("os");

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get("/", function (req, res) {
  res.sendFile(path.join(__dirname + "/public/index.html"));
});

// Send Style, do not change
app.get("/style.css", function (req, res) {
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname + "/public/style.css"));
});

// Send obfuscated JS, do not change
app.get("/index.js", function (req, res) {
  fs.readFile(
    path.join(__dirname + "/public/index.js"),
    "utf8",
    function (err, contents) {
      const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {
        compact: true,
        controlFlowFlattening: true,
      });
      res.contentType("application/javascript");
      res.send(minimizedContents._obfuscatedCode);
    }
  );
});

//Respond to POST requests that upload files to uploads/ directory
app.post("/upload", function (req, res) {
  if (!req.files) {
    return res.status(400).send("No files were uploaded.");
  }

  const uploadFile = req.files.uploadFile;

  if (fs.readdirSync(__dirname + "/uploads/").includes(uploadFile.name)) {
    console.log("File already exists. No upload")
    res.redirect("/");
    return;
  }


  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv("uploads/" + uploadFile.name, function (err) {
    if (err) {
      res.redirect("/");
      return;
    }

    // res.status(200).send({ data: `${uploadFile.name} uploaded successfully.` });
    res.redirect("/");
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get("/uploads/:name", function (req, res) {
  fs.stat("uploads/" + req.params.name, function (err, stat) {
    if (err == null) {
      res.sendFile(path.join(`${__dirname}/uploads/${req.params.name.split(" ").join("_")}`));
    } else {
      console.log("Error in file downloading route: " + err);
      res.send("");
    }
  });
});

//******************** Your code goes here ********************

let db;

/**
 * Modify global db variable to create mysql connection and perform setup
 * @param {*} user Database username
 * @param {*} password Database password
 * @param {*} database Database name
 */
const connect = async (user, password, database) => {
  try {
    db = await mysql.createConnection({
      host: 'dursley.socs.uoguelph.ca',
      user,
      password,
      database
    });
    // Create FILE table
    await db.execute("CREATE TABLE IF NOT EXISTS FILE(gpx_id INT AUTO_INCREMENT PRIMARY KEY,file_name VARCHAR(60) NOT NULL,ver DECIMAL(2,1) NOT NULL,creator VARCHAR(256) NOT NULL);");

    // Create ROUTE table
    await db.execute("CREATE TABLE IF NOT EXISTS ROUTE(route_id INT AUTO_INCREMENT PRIMARY KEY,route_name VARCHAR(256),route_len FLOAT(15,7) NOT NULL,gpx_id INT NOT NULL, CONSTRAINT FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE);");

    // Create POINT table
    await db.execute("CREATE TABLE IF NOT EXISTS POINT(point_id INT AUTO_INCREMENT PRIMARY KEY,point_index INT NOT NULL,latitude DECIMAL(11,7) NOT NULL,longitude DECIMAL(11,7) NOT NULL,point_name VARCHAR(256),route_id INT NOT NULL, CONSTRAINT FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE);");
  } catch (e) {
    console.log(e);
  }
};

// My C Library functions
const gpx = ffi.Library('./libgpxparser.so', {
  'getFileTableData': ['string', ['string']],
  'getRoutes': ['string', ['string']],
  'getTracks': ['string', ['string']],
  'getWaypoints': ['string', ['string']],
  'renameComponent': ['void', ['string', "string", 'string', "int"]],
  'JSONtoGPXFile': ['int', ['string', "string"]],
  'addRouteToFile': ['void', ['string', "string"]],
  'addWaypointToRoute': ['void', ['string', "string", "int"]],
  'getPathsBetween': ['string', ['string', "float", "float", "float", "float", "float"]],
});

const createGPX = (fileName) => ({
  ...JSON.parse(gpx.getFileTableData(fileName)),
  waypoints: JSON.parse(gpx.getWaypoints(fileName)),
  routes: JSON.parse(gpx.getRoutes(fileName)),
  tracks: JSON.parse(gpx.getTracks(fileName)),
});

const getFileNames = () => fs.readdirSync(__dirname + "/uploads/");

const createDocs = () => getFileNames().filter((name) => name.slice(-4) === ".gpx").map(e => {
  const fileName = `${__dirname}/uploads/${e}`;
  return ({
    name: e,
    ...createGPX(fileName)
  });
}).filter(e => e.creator);

app.get("/docs/", function (req, res) {
  res.send({
    data: createDocs()
  });
});

app.get("/docs/:name", (req, res) => {
  res.send({
    data: getFileNames().filter((name) => name.slice(-4) === ".gpx").map(e => {
      const fileName = `${__dirname}/uploads/${e}`;

      return ({
        name: e,
        ...createGPX(fileName)
      });
    }).filter(e => e.creator).filter(({ name }) => name.includes(req.params.name))
  });
});

app.post("/login", async (req, res) => {
  const { user, password, database } = req.body;

  await connect(user, password, database);

  if (db) {
    return res.status(200).send({ data: "Success" });
  }

  return res.status(400).send({ data: "Failed" });

});

app.post("/store_files", async (req, res) => {
  const { docs } = req.body;

  try {
    // Get existing file names
    const [data] = await db.execute("SELECT file_name FROM FILE;");

    // Check if there are any new names (ones not inside our DB)
    const existingNames = data.map(({ file_name }) => file_name);
    const newNames = docs.filter(({ name }) => !existingNames.includes(name)).map(({ name }) => name);

    const filteredDocs = docs.filter(({ name }) => newNames.includes(name))

    // If there is at least one new name, complete insertion
    filteredDocs.forEach(async ({ name, creator, version, routes = [] }) => {

      // Insert file into table
      const newFile = await db.execute(`INSERT INTO FILE ( file_name, ver, creator) VALUES ('${name}',${version},'${creator}');`);

      // Get GPX_ID from the file we just inserted
      // Alternatively, can grab new ID from newData
      const [[{ gpx_id }]] = await db.execute(`SELECT gpx_id FROM FILE WHERE file_name='${name}'`);

      routes.forEach(async (route) => {

        // Insert route into table
        await db.execute(`INSERT INTO ROUTE ( route_name, route_len, gpx_id) VALUES ('${route.name}',${route.len},${gpx_id});`);

        const [[{ route_id }]] = await db.execute(`SELECT route_id FROM ROUTE WHERE route_name='${route.name}'`);
        route?.waypoints?.forEach(async (point, index) => {

          // Insert point into table
          await db.execute(`INSERT INTO POINT (point_index, latitude,longitude, point_name,route_id) VALUES (${index},${point.lat},${point.lon},'${point.name}',${route_id});`);

        });
      });
    });

  } catch (e) {
    console.log(e);
  }

  return res.status(200).send({ data: "Success" });
});

app.get("/db_status", async (req, res) => {
  try {
    const [[files]] = await db.execute("SELECT COUNT(*) FROM FILE");
    const [[routes]] = await db.execute("SELECT COUNT(*) FROM ROUTE");
    const [[points]] = await db.execute("SELECT COUNT(*) FROM POINT");

    return res.status(200).send({ data: `Database has ${files["COUNT(*)"]} files, ${routes["COUNT(*)"]} routes, ${points["COUNT(*)"]} points` });
  } catch (e) {
    return res.status(200).send({ data: e });
  }
});

app.get("/db/routes", async (req, res) => {
  const { sort } = req.query;
  const methods = { name: "route_name", length: "route_len" };
  try {
    const [routes] = await db.execute(`
    SELECT * FROM ROUTE
    ORDER BY ${methods[sort]} ASC;
    `);

    return res.status(200).send({ data: routes });
  } catch (e) {
    return res.status(200).send({ data: e });
  }
});
app.get("/db/routes/:name", async (req, res) => {
  const { name } = req.params;
  const { sort } = req.query;
  const methods = { name: "route_name", length: "route_len" };
  try {

    const [routes] = await db.execute(`
      SELECT ROUTE.route_id,ROUTE.route_name,ROUTE.route_len,FILE.file_name FROM FILE,ROUTE
      WHERE (FILE.gpx_id = ROUTE.gpx_id and FILE.file_name='${name}')
      ORDER BY ${methods[sort]} ASC
    ;`);

    return res.status(200).send({ data: routes });
  } catch (e) {
    return res.status(200).send({ data: e });
  }
});
app.get("/db/route/points", async (req, res) => {
  const { file, route } = req.query;
  try {

    const [points] = await db.execute(`
      SELECT POINT.point_index,POINT.latitude,POINT.longitude,POINT.point_name FROM FILE,ROUTE,POINT
      WHERE (FILE.gpx_id = ROUTE.gpx_id and ROUTE.route_id = POINT.route_id and FILE.file_name='${file}' and ROUTE.route_name='${route}')
      ORDER BY POINT.point_index ASC
    ;`);

    return res.status(200).send({ data: points });
  } catch (e) {
    return res.status(200).send({ data: e });
  }
});
app.get("/db/file/points", async (req, res) => {
  const { file, sort } = req.query;
  const methods = { name: "route_name", length: "route_len" };

  try {

    const [points] = await db.execute(`
    SELECT POINT.point_index,POINT.latitude,POINT.longitude,POINT.point_name,ROUTE.route_name FROM FILE,ROUTE,POINT
    WHERE (FILE.gpx_id = ROUTE.gpx_id and ROUTE.route_id = POINT.route_id and FILE.file_name='${file}')
    ORDER BY POINT.point_index ASC, ROUTE.${methods[sort]} ASC
    ;`);

    return res.status(200).send({ data: _.groupBy(points, "route_name") });
  } catch (e) {
    return res.status(400).send({ data: e });
  }
});

app.post("/clear_tables", async (req, res) => {
  await db.execute("DELETE FROM POINT;")
  await db.execute("DELETE FROM ROUTE;")
  await db.execute("DELETE FROM FILE;")
  return res.status(200).send({ data: "Success" });
});

app.get("/db/nroutes", async (req, res) => {
  const { file, sort, n, spec } = req.query;
  const methods = { name: "route_name", length: "route_len" };
  const specMethods = { shortest: "ASC", longest: "DESC" };

  try {
    const [routes] = await db.execute(`SELECT FILE.file_name,ROUTE.route_name,ROUTE.route_len FROM FILE,ROUTE
    WHERE (FILE.file_name='${file}' and ROUTE.gpx_id = FILE.gpx_id)
    ORDER BY ROUTE.route_len ${specMethods[spec]}
    LIMIT ${n}
    ;`);

    return res.status(200).send({ data: _.sortBy(routes, methods[sort]) });
  } catch (e) {
    return res.status(400).send({ data: e });
  }
});

app.post("/rename", (req, res) => {
  const [component, index] = req.body.component.split(" ");

  gpx.renameComponent(correctPath(req.body.file), req.body.name, component.toLowerCase(), +index - 1);

  res.status(200).send({ data: "Monkey poopoo" });
});

app.post("/create", async (req, res) => {
  const { name, version, creator } = req.body;

  if (getFileNames().includes(name) || name?.slice(-4) !== ".gpx" || !name || !version || !creator || version !== 1.1)
    return res.status(400).send({ data: "Bad arguments" })

  if (gpx.JSONtoGPXFile(`./uploads/${name}`, JSON.stringify({ version, creator })))
    return res.status(400).send({ data: "Error creating file." });

  return res.status(200).send({ data: "Success" });
});

const correctPath = (name) => `./uploads/${name}`;

app.post("/add_route/", async (req, res) => {
  const { name, file, waypoints, index } = req.body;

  if (name === null || file === null || index === null)
    return res.status(400).send({ data: "Bad request" });

  gpx.addRouteToFile(correctPath(file), JSON.stringify({ name }));
  waypoints.forEach(({ lat, lon }) => gpx.addWaypointToRoute(correctPath(file), JSON.stringify({ lat, lon }), index));

  return res.status(200).send({ data: "Stinky" });
});

app.get("/paths", (req, res) => {
  const { lat1, lat2, lon1, lon2, delta } = req.query;

  const docs = createDocs();

  const paths = docs.map(e => JSON.parse(gpx.getPathsBetween(correctPath(e.name), lat1, lon1, lat2, lon2, delta)));
  const filtered = paths.filter(({ routes, tracks }) => (routes.length > 0 || tracks.length > 0));
  const tracks = filtered.map(({ tracks }) => tracks).filter(e => e.length > 0);
  const routes = filtered.map(({ routes }) => routes).filter(e => e.length > 0);

  return res.send({ data: { routes, tracks } });
})

app.listen(portNum);
console.log("Running app at localhost: " + portNum);
