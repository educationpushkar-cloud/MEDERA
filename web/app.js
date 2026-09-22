const API = "";   // same-origin: empty prefix

// ---------- helpers ----------
async function postForm(path, params) {
    const body = new URLSearchParams(params).toString();
    const res = await fetch(API + path, {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body
    });
    return res.json();
}

async function getJson(path) {
    const res = await fetch(API + path);
    return res.json();
}

function render(el, data) {
    const node = document.getElementById(el);
    if (typeof data === "string") node.innerHTML = data;
    else node.innerHTML = "<pre>" + JSON.stringify(data, null, 2) + "</pre>";
}

// ---------- hospital search ----------
async function searchHospitals() {
    const city = document.getElementById("searchCity").value.trim().toUpperCase();
    const area = document.getElementById("searchArea").value.trim().toUpperCase();
    if (!city || !area) return render("searchResults", "Enter both city and area.");

    const data = await getJson(
        `/api/hospitals?city=${encodeURIComponent(city)}&area=${encodeURIComponent(area)}`
    );
    if (!data.length) return render("searchResults", "No hospitals found.");
    render("searchResults", data);
}

// ---------- emergency nearest ----------
async function findNearest() {
    const lat = parseFloat(document.getElementById("emLat").value);
    const lon = parseFloat(document.getElementById("emLon").value);
    if (isNaN(lat) || isNaN(lon)) return render("nearestResults", "Enter lat and lon.");
    const data = await getJson(`/api/emergency/nearest?lat=${lat}&lon=${lon}&k=3`);
    render("nearestResults", data);
}

// ---------- emergency request ----------
async function raiseEmergency() {
    const patientId = parseInt(document.getElementById("emPatientId").value);
    const lat = parseFloat(document.getElementById("emReqLat").value);
    const lon = parseFloat(document.getElementById("emReqLon").value);
    if (isNaN(patientId) || isNaN(lat) || isNaN(lon))
        return render("emergencyState", "Fill all fields.");

    const data = await postForm("/api/emergency/request",
        { patient_id: patientId, lat, lon });

    // Auto-advance through the FSM to Accepted to demo the flow.
    if (data.id) {
        const accepted = await postForm(`/api/emergency/${data.id}/accept`, {});
        render("emergencyState", accepted);
    } else {
        render("emergencyState", data);
    }
}

// ---------- appointments ----------
async function bookAppointment() {
    const patientId  = parseInt(document.getElementById("bkPatient").value);
    const hospitalId = parseInt(document.getElementById("bkHospital").value);
    const doctorId   = parseInt(document.getElementById("bkDoctor").value);
    if (isNaN(patientId) || isNaN(hospitalId) || isNaN(doctorId))
        return render("bookResult", "Fill all fields.");

    const data = await postForm("/api/appointments",
        { patient_id: patientId, hospital_id: hospitalId, doctor_id: doctorId });
    render("bookResult", data);
}

async function loadAppointments() {
    const patientId = parseInt(document.getElementById("myPatient").value);
    if (isNaN(patientId)) return render("appointmentList", "Enter patient ID.");
    const data = await getJson(`/api/appointments?patient_id=${patientId}`);
    render("appointmentList", data);
}

// ---------- staff ----------
let currentStaff = null;

async function staffLogin() {
    const staffId = document.getElementById("staffId").value.trim();
    const password = document.getElementById("staffPass").value;
    if (!staffId || !password) return render("staffResult", "Enter both fields.");

    const data = await postForm("/api/staff/login",
        { staff_id: staffId, password });

    render("staffResult", data);

    if (data.status === "ok") {
        currentStaff = data.staff;
        document.getElementById("bedUpdatePanel").style.display = "block";
        document.getElementById("bedHospital").value = data.staff.hospital_id;
    }
}

async function updateBeds() {
    if (!currentStaff) return render("bedResult", "Login first.");
    const id = parseInt(document.getElementById("bedHospital").value);
    const beds = parseInt(document.getElementById("bedCount").value);
    if (isNaN(id) || isNaN(beds)) return render("bedResult", "Fill all fields.");

    const data = await postForm("/api/hospital/update-beds",
        { id, available_beds: beds, staff_id: currentStaff.id });
    render("bedResult", data);
}