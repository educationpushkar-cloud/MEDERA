// Talks to the endpoints exposed by src/server.cpp. Requires the server to
// be running (see the setup note at the top of server.cpp).

document.getElementById("searchForm").addEventListener("submit", async (e) => {
  e.preventDefault();
  const city = document.getElementById("cityInput").value.trim().toUpperCase();
  const area = document.getElementById("areaInput").value.trim().toUpperCase();

  const res = await fetch(`/api/hospitals?city=${encodeURIComponent(city)}&area=${encodeURIComponent(area)}`);
  const hospitals = await res.json();

  const container = document.getElementById("searchResults");
  container.innerHTML = "";

  if (hospitals.length === 0) {
    container.innerHTML = "<p>No hospitals found for that city/area.</p>";
    return;
  }

  hospitals.forEach((h) => {
    const row = document.createElement("div");
    row.className = "hospital-row";
    row.innerHTML = `
      <div>
        <div class="name">${h.name}</div>
        <div class="meta">${h.city} &middot; ${h.area}</div>
      </div>
      <div class="meta">
        Beds: ${h.available_beds}/${h.total_beds} &middot; Rating: ${h.rating}&#9733;
      </div>`;
    container.appendChild(row);
  });
});

document.getElementById("emergencyForm").addEventListener("submit", async (e) => {
  e.preventDefault();
  const lat = document.getElementById("latInput").value;
  const lon = document.getElementById("lonInput").value;

  const res = await fetch(`/api/emergency/nearest?lat=${lat}&lon=${lon}&k=3`);
  const ranked = await res.json();

  const container = document.getElementById("emergencyResults");
  container.innerHTML = "";

  if (ranked.length === 0) {
    container.innerHTML = "<p>No hospitals with an available bed were found nearby.</p>";
    return;
  }

  ranked.forEach((rh, i) => {
    const row = document.createElement("div");
    row.className = "hospital-row";
    row.innerHTML = `
      <div>
        <div class="name">${i + 1}. ${rh.hospital.name}</div>
        <div class="meta">${rh.distance_km.toFixed(2)} km away &middot; score ${rh.score.toFixed(2)}</div>
      </div>
      <div class="meta">Beds: ${rh.hospital.available_beds}/${rh.hospital.total_beds}</div>`;
    container.appendChild(row);
  });
});
