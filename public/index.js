/******************************************************
                      HTML TEMPLATES
*******************************************************/

const createTableElement = ({
  id,
  name = "",
  version = "N/A",
  creator = "N/A",
  numWaypoints = null,
  numRoutes = null,
  numTracks = null,
}) =>
  `
    <tr id="${id}" class="file-table-element">
        <td><a href="/uploads/${name}" target="_blank">${name}</a></td>
        <td>${version}</td>
        <td>${creator}</td>
        <td>${numWaypoints}</td>
        <td>${numRoutes}</td>
        <td>${numTracks}</td>
    </tr>
`;
const createDetailedElement = ({
  id,
  numPoints = null,
  len = null,
  loop = null,
  name = "",
  type = "N/A"
}) => `
    <div id="${id}" class="detailed-table-element">
    <div class="text-container">
      <i class="bi bi-plus-circle plus-open-other"></i>
        <div>
          <h3>${type} - ${name}</h3>
          <p>Points: ${numPoints}</p>
          <p>Length: ${len}</p>
          <p>Loop: ${loop}</p>
        </div>
      </div>
      <div class="other-container hidden">

      </div>
    </div>
`;

const createDetailedTableElement = ({
  id,
  numPoints = null,
  len = null,
  loop = null,
  name = "",
  type = "N/A"
}) => `
  <tr id="${id}" class="file-table-element">
    <td>${type}</td>
    <td>${name}</td>
    <td>${numPoints}</td>
    <td>${len}m</td>
    <td>${loop}</td>
  </tr>
`;
const createRouteWaypointForm = () => `
  <div class="form-group">
    <label>Latitude</label>
    <input class="form-control" type="text" placeholder="Latitude">

    <label>Longitude</label>
    <input class="form-control" type="text" placeholder="Longitude">
  </div>
`;

const createOption = ({ name }) => `
  <option value="${name}">${name}</option>
`;

const EMPTY_TABLE_5 = `
  <tr>
    <td>No Data</td>
    <td>No Data</td>
    <td>No Data</td>
    <td>No Data</td>
    <td>No Data</td>
  </tr>
`;

/******************************************************
                      HELPER FUNCTIONS
*******************************************************/

const post = async (url, json = {}) => {
  const { data } = await $.ajax({
    url: url,
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data:
      JSON.stringify(json)
  });
  return data;
}

const get = async (url, json = {}) => {
  const { data } = await $.ajax({
    url: url,
    type: "GET",
    contentType: "application/json",
    dataType: "json",
    data: json
  });
  return data;
}

/**
 * yeah, you like that?
 */
const GOOBA_KEKE_FEFE_KIKA_GOTTI_GUMMO_ZAZA_BEBE_BILLY_TUTU = async () => {
  try {
    await post("/clear_tables");
    const data = await get("/docs");
    await post("/store_files", { docs: data });
  } catch (e) {
    alert("Fellas iffy, uh, Blicky got the stiffy, uh");
  }
}


const populateGPXTable = async () => {
  const data = await get("/docs");

  if (data.length) {
    $("#store-files-button").removeClass("hidden");
    $("#file-log").html("");
    data.forEach((e, index) => {

      const fileId = `file-${index}`
      $("#file-log").append(
        createTableElement({
          id: fileId,
          ...e
        })
      );

      // Create event handlers
      $(document).off("click", `#${fileId}`);
      $(document).on("click", `#${fileId}`, function () {
        $("#gpx-select").val(e.name);
        updateDetailedTable(e.name);
      });

      $("#gpx-select").append(
        createOption({
          id: `option-${index}`,
          ...e
        })
      );
      $("#db-specific-routes-file").append(
        createOption({
          id: `db-route-option-${index}`,
          ...e
        })
      );
      $("#db-specific-points-file").append(
        createOption({
          id: `db-point-option-${index}`,
          ...e
        })
      );
      $("#db-file-points-file").append(
        createOption({
          id: `db-file-point-option-${index}`,
          ...e
        })
      );
      $("#db-n-routes-file").append(
        createOption({
          id: `db-n-routes-option-${index}`,
          ...e
        })
      );
    });

    return;
  }
  $("#store-files-button").addClass("hidden");
  $("#file-log").append(
    `
      <tr>
          <td>No Files</td>
          <td>No Files</td>
          <td>No Files</td>
          <td>No Files</td>
          <td>No Files</td>
          <td>No Files</td>
      </tr>
  `);
}
const updatePathsDisplay = ({ routes = [], tracks = [] }) => {
  $("#paths-output").html("");

  if (routes.length == 0 && tracks.length == 0) {
    $("#paths-output").append(EMPTY_TABLE_5);
    return;
  }

  [].concat(...routes).forEach((e, index) => {
    $("#paths-output").append(
      createDetailedTableElement({ id: `paths-route-${index}`, type: `Route ${index + 1}`, ...e })
    );
  });

  [].concat(...tracks).forEach((e, index) => {
    $("#paths-output").append(
      createDetailedTableElement({ id: `paths-track-${index}`, type: `Track ${index + 1}`, ...e })
    )
  }
  );
};

const updateDetailedTable = async (name) => {
  const data = await get(`/docs/${name}`);

  $("#detailed-view").html("");

  if (data[0].routes.length == 0 && data[0].tracks.length == 0) {
    $("#detailed-view").append(EMPTY_TABLE_5);
    return;
  }

  data.forEach(({ routes = [], tracks = [] }) => {
    // Add all routes
    routes.forEach((e, index) => {

      // Create element
      $("#detailed-view").append(
        createDetailedTableElement({ id: `detailed-route-${index}`, type: `Route ${index + 1}`, ...e })
      );

      // Create event handlers
      $(document).off("click", `#detailed-route-${index}`);
      $(document).on("click", `#detailed-route-${index}`, function () {
        alert(e.otherData.map(({ name, value }) => `${name}: ${value}\n`).join(", "))
      });
    }
    );

    // Add all tracks
    tracks.forEach((e, index) => {
      // Create element
      $("#detailed-view").append(
        createDetailedTableElement({ id: `detailed-track-${index}`, type: `Track ${index + 1}`, ...e })
      );

      // Create event handlers
      $(document).off("click", `#detailed-track-${index}`);
      $(document).on("click", `#detailed-track-${index}`, function () {
        alert(e.otherData.map(({ name, value }) => `${name}: ${value}\n`).join(", "))
      });
    });
  });
};

$(document).ready(function () {
  populateGPXTable();

  $("#gpx-select").change((e) => {
    e.preventDefault();
    updateDetailedTable(e.target.value);
  });

  $("#form-rename").submit(async (e) => {
    e.preventDefault();

    const file = $("#gpx-select").val();

    await post("/rename", {
      component: $("#input-rename-component").val(),
      name: $("#input-rename-name").val(),
      file
    });

    GOOBA_KEKE_FEFE_KIKA_GOTTI_GUMMO_ZAZA_BEBE_BILLY_TUTU();

    updateDetailedTable(file);
  });

  $("#form-create").submit(async (e) => {
    e.preventDefault();
    const name = $("#input-create-name").val();
    const version = $("#input-create-version").val();
    const creator = $("#input-create-creator").val();

    if (name.slice(-4) !== ".gpx") {
      alert("New GPX name must have .gpx extension");
      return;
    }

    await post("/create", {
      name: name,
      version: +version,
      creator: creator
    });

    GOOBA_KEKE_FEFE_KIKA_GOTTI_GUMMO_ZAZA_BEBE_BILLY_TUTU();

    populateGPXTable();

  });
  $("#btn-add-waypoint").click((e) => {
    $("#add-waypoints").append(createRouteWaypointForm())
  });
  $("#form-add-route").submit(async (e) => {
    e.preventDefault();

    let waypoints_tmp = [];

    // Populate waypoints 2D array with children of the form
    $("#add-waypoints").children().each(function (i) {
      waypoints_tmp[i] = [];
      $(this).children("input").each(function (j) {
        waypoints_tmp[i][j] = $(this).val();
      })
    });

    // Convert to array of objects because my function expects this type
    const waypoints = waypoints_tmp.map(([lat, lon]) => ({ name: "", lat: +lat, lon: +lon }));

    if (waypoints.filter(({ lat, lon }) => isNaN(lat) || isNaN(lon)).length) {
      alert("Inputs must be of type Number");
      return;
    }

    const name = $("#input-add-route-name").val();
    const file = $("#gpx-select").val();

    if (file === "None") {
      alert("Can't add route to unknown GPX doc");
      return;
    }

    const [data] = await get(`/docs/${file}`);

    const index = data.routes.length - 1;
    await post("/add_route", {
      name,
      waypoints,
      file,
      index: index < 0 ? 0 : index
    });

    GOOBA_KEKE_FEFE_KIKA_GOTTI_GUMMO_ZAZA_BEBE_BILLY_TUTU();
    updateDetailedTable(file);
  })
  $("#form-path-between").submit(async (e) => {
    e.preventDefault();

    const lat1 = +$("#input-path-lat-1").val();
    const lat2 = +$("#input-path-lat-2").val();
    const lon1 = +$("#input-path-lon-1").val();
    const lon2 = +$("#input-path-lon-2").val();
    const delta = +$("#input-path-delta").val();

    if (isNaN(lat1) || isNaN(lon1) || isNaN(lat2) || isNaN(lon2) || isNaN(delta)) {
      alert("Inputs must be of type Number");
      return;
    }

    const paths = await get("/paths", {
      lat1, lon1, lat2, lon2, delta
    });

    updatePathsDisplay(paths);

  });
  $("#form-find-length").submit(async (e) => {
    e.preventDefault();

    $("#lengths-output").html("");

    const route = +$("#input-find-length-route").val();
    const track = +$("#input-find-length-track").val();
    const delta = 10;

    if (isNaN(route) || isNaN(track)) {
      alert("Inputs must be of type Number");
      return;
    }

    const data = await get("/docs");

    let routeCount = 0;
    let trackCount = 0;

    // This entire block is black magic basically
    const result = data.map(({ routes, tracks }) => {
      const routeList = routes.filter(({ len }) => (Math.abs(len - route) <= delta));
      const trackList = tracks.filter(({ len }) => (Math.abs(len - track) <= delta));

      routeList.forEach((e) => $("#lengths-output").append(createDetailedTableElement({ id: `length-${++routeCount}`, type: `Route ${routeCount}`, ...e })));
      trackList.forEach((e) => $("#lengths-output").append(createDetailedTableElement({ id: `length-${++trackCount}`, type: `Track ${trackCount}`, ...e })));

      return [...routeList, ...trackList];
    }).filter(e => e !== null && e?.length);

    if (result.length == 0) {
      $("#lengths-output").append(EMPTY_TABLE_5);
    }
  })

  $("#form-login").submit(async (e) => {
    e.preventDefault();

    try {
      await post("/login", {
        user: $("#login-user").val(),
        password: $("#login-password").val(),
        database: $("#login-database").val()
      });
      alert("Login Success");
      $("#login").toggleClass("hidden");
      $("#db-panel").toggleClass("hidden");
    } catch (e) {
      alert("Login Failed");
    }
  });

  $("#store-files-button").click(async (e) => {
    try {
      await post("/clear_tables");
      const data = await get("/docs");
      await post("/store_files", { docs: data });
    } catch (e) {
      alert("Could not store files");
    }
  });
  $("#clear-tables-button").click(async (e) => {
    try {
      await post("/clear_tables");
    } catch (e) {
      alert("Could not clear tables");
    }
  });

  $("#display-status-button").click(async (e) => {
    try {
      const data = await get("/db_status");
      alert(data);
    } catch (e) {
      alert("Error getting DB status");
    }
  });

  const createTableRoute = ({ route_id, route_name, route_len, gpx_id }) => `
  <tr>
  <td>${route_id}</td>
    <td>${route_name}</td>
    <td>${route_len}</td>
    <td>${gpx_id}</td>
    </tr>
    `;
  const createSpecificTableRoute = ({ file_name, route_name, route_len }) => `
    <tr>
    <td>${file_name}</td>
    <td>${route_name}</td>
    <td>${route_len}</td>
    </tr>
    `;

  $("#db-all-routes-sort").change(async () => {
    const sortingMethod = $("#db-all-routes-sort").val();

    // Do nothing
    if (sortingMethod === "none") {
      $("#db-all-routes-table").html(createTableRoute({ route_id: "None", route_name: "None", route_len: "None", gpx_id: "None" }));
      return;
    }

    const routes = await get("/db/routes", { sort: sortingMethod });

    $("#db-all-routes-table").html("");

    // Case where there are routes to display
    if (routes.length) {
      routes.forEach((e) => $("#db-all-routes-table").append(createTableRoute(e)));
      return;
    }

    // Case where we were not able to fetch any routes
    $("#db-all-routes-table").html(createTableRoute({ route_id: "None", route_name: "None", route_len: "None", gpx_id: "None" }));
  });
  $("#db-specific-routes-form").submit(async (e) => {
    e.preventDefault();
    const sortingMethod = $("#db-specific-routes-sort").val();
    const fileName = $("#db-specific-routes-file").val();

    // Do nothing
    if (sortingMethod === "none" || fileName === "none") {
      $("#db-specific-routes-table").html(createSpecificTableRoute({ file_name: "None", route_name: "None", route_len: "None" }));
      return;
    }

    const routes = await get(`/db/routes/${fileName}`, { sort: sortingMethod });

    $("#db-specific-routes-table").html("");

    // Case where there are routes to display
    if (routes.length) {
      routes.forEach((rt) => $("#db-specific-routes-table").append(createSpecificTableRoute(rt)));
      return;
    }

    // Case where we were not able to fetch any routes
    $("#db-specific-routes-table").html(createSpecificTableRoute({ file_name: "None", route_name: "None", route_len: "None" }));
  });

  const DEFAULT_OPTION = `<option value="none" selected>None</option>`;

  $("#db-specific-points-file").change(async (e) => {
    const [{ routes }] = await get(`/docs/${e.target.value}`);

    $("#db-specific-points-route").html(DEFAULT_OPTION);

    routes?.forEach(({ name }) => $("#db-specific-points-route").append(createOption({ name })));
  });

  const createTablePoint = ({ point_index, point_name, latitude, longitude }) => `
<tr>
<td>${point_index}</td>
<td>${point_name}</td>
<td>${latitude}</td>
<td>${longitude}</td>
</tr>
`;
  $("#db-specific-points-form").submit(async (e) => {
    e.preventDefault();
    const route = $("#db-specific-points-route").val();
    const file = $("#db-specific-points-file").val();

    if (route === "none" || file === "none")
      return;

    $("#db-specific-points-table").html("");

    const points = await get(`/db/route/points`, { file, route });

    if (points.length) {
      points?.forEach((pt) => $("#db-specific-points-table").append(createTablePoint(pt)));
      return;
    }

    // Case where we get no points
    $("#db-specific-points-table").html(createTablePoint({ point_index: "None", point_name: "None", latitude: "None", longitude: "None" }));
  });

  const createNewTableElement = (obj) => {
    return `<tr>${Object.entries(obj).map(([key, val]) => `<td>${val}</td>`).join("")}</tr>`;
  }

  $("#db-file-points-form").submit(async (e) => {
    e.preventDefault();

    const sortingMethod = $("#db-file-points-sort").val();
    const file = $("#db-file-points-file").val();

    if (file === "none")
      return;

    const points = await get("/db/file/points", { file, sort: sortingMethod });

    $("#db-file-points-table").html("");

    if (Object.entries(points).length) {
      Object.entries(points).forEach(([key, val], index) => {
        let name = key;
        if (name === "None")
          name = `Unnamed Route ${index}`;

        val.forEach(({ point_index, point_name, latitude, longitude }) => $("#db-file-points-table").append(`<tr>${createNewTableElement({ name, point_index, point_name, latitude, longitude })}</tr>`));
      });
      return;
    }

    $("#db-file-points-table").html(`
      <tr>
        <td>None</td>
        <td>None</td>
        <td>None</td>
        <td>None</td>
        <td>None</td>
      </tr>`
    );
  });

  $("#db-n-routes-form").submit(async (e) => {
    e.preventDefault();

    const file = $("#db-n-routes-file").val();
    const n = $("#db-n-routes-n").val();
    const spec = $("#db-n-routes-spec").val();
    const sortingMethod = $("#db-n-routes-sort").val();

    if (file === "none")
      return;

    if (n < 0) {
      alert("N Must be positive");
      return;
    }
    $("#db-n-routes-table").html(``);
    const routes = await get("/db/nroutes", { file, n, spec, sort: sortingMethod });

    if (routes.length) {
      routes.forEach(({ file_name, route_name, route_len }) => $("#db-n-routes-table").append(createNewTableElement({ file_name, route_name, route_len, })));
      return;
    }
  });

  $("#db-n-routes-table").html(`
      <tr>
        <td>None</td>
        <td>None</td>
        <td>None</td>
      </tr>`
  );

});