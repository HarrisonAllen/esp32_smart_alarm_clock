var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);

var max_volume = 21;
var min_volume = 0;
var alarmData;      

function onload(event) {
    initWebSocket();
}

function getData() {
    websocket.send("getData");
}

function sendData() {
    console.log("Sending data");
    websocket.send(JSON.stringify(alarmData));
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    getData();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function setAlarmData(data) {
    var hour = Number(data[0]);
    var isMorning = data[2] == "AM";
    if (!isMorning) {
        if (hour != 12) {
            hour += 12;
        }
    } else {
        if (hour == 12) {
            hour = 0;
        }
    }
    var minute = Number(data[1]);
    var enabled = data[3] == "On";
    var repeat = data[4] == "Repeat"

    alarmData[0]["alarm"]["enabled"] = enabled;
    alarmData[0]["alarm"]["hour"] = hour;
    alarmData[0]["alarm"]["minute"] = minute;
    alarmData[0]["alarm"]["currentHour"] = hour;
    alarmData[0]["alarm"]["currentMinute"] = minute;
    alarmData[0]["alarm"]["repeat"] = repeat;

    sendData();
    window.location.reload();
}

function setSnoozeData(data) {
    var duration = Number(data[0].split(" ")[0]);
    var limit = Number(data[1].split("x")[0]);
    var enabled = data[2] == "On";

    alarmData[0]["snooze"]["enabled"] = enabled;
    alarmData[0]["snooze"]["duration"] = duration;
    alarmData[0]["snooze"]["limit"] = limit;
    alarmData[0]["snooze"]["remaining"] = limit;

    sendData();
    window.location.reload();
}

function setVolumeData(data) {
    var volume = Number(data[0]);

    alarmData[0]["sound"]["volume"] = volume;

    sendData();
    window.location.reload();
}

function setAudioData(data) {
    var file = "/audio/" + data[0];

    alarmData[0]["sound"]["file"] = file;

    sendData();
    window.location.reload();
}

function setTimezoneData(data) {
    var splitData = data[0].split(":");
    var hour = Number(splitData[0]);
    var minute = Number(splitData[1]);
    minute = hour >= 0 ? minute : -minute;
    var timeOffset = hour * 60 * 60 + minute * 60;

    alarmData[0]["timeOffset"] = timeOffset;

    sendData();
    window.location.reload();
}

function snooze() {
    console.log("snooze");
    websocket.send("snooze");
}

function stop() {
    console.log("stop");
    websocket.send("stop");
}

function updateAlarmStatus(alarmActive, snoozeActive, snoozesRemaining, snoozeEnabled) {
    var alarmStatus = document.getElementById("alarmStatus");
    var snoozeButton = document.getElementById("snoozeButton");
    var stopButton = document.getElementById("stopButton");
    if (alarmActive) {
        alarmStatus.textContent = "🚨 ALARM 🚨";
        stopButton.enabled = true;
    } else if (snoozeActive) {
        alarmStatus.textContent = "😴 x" + snoozesRemaining + " left";
        stopButton.enabled = true;
    } else {
        alarmStatus.textContent = "";
        stopButton.enabled = false;
    }
    snoozeButton.enabled = alarmActive && snoozeEnabled && !snoozeActive && (snoozesRemaining > 0);
}

function onMessage(event) {
    console.log(event.data);
    alarmData = JSON.parse(event.data);

    // Alarm settings
    var alarmHour = alarmData[0]["alarm"]["hour"];
    var ampm;
    if (alarmHour < 12) {
        ampm = "AM";
        if (alarmHour == 0) {
            alarmHour = 12;
        }
    } else {
        ampm = "PM";
        if (alarmHour != 12) {
            alarmHour -= 12;
        }
    }
    alarmHour = String(alarmHour);
    var alarmMinute = String(alarmData[0]["alarm"]["minute"]).padStart(2, '0');
    var alarmEnabled = alarmData[0]["alarm"]["enabled"] ? "On" : "Off";
    var alarmRepeat = alarmData[0]["alarm"]["repeat"] ? "Repeat" : "Once";
    alarmSelect.locatePosition(0, hourArray.indexOf(alarmHour));
    alarmSelect.locatePosition(1, minArray.indexOf(alarmMinute));
    alarmSelect.locatePosition(2, amArray.indexOf(ampm));
    alarmSelect.locatePosition(3, enableArray.indexOf(alarmEnabled));
    alarmSelect.locatePosition(4, repeatArray.indexOf(alarmRepeat));

    // Snooze settings
    var snoozeDuration = String(alarmData[0]["snooze"]["duration"]).padStart(2, '0') + " min";
    var snoozeLimit = String(alarmData[0]["snooze"]["limit"]) + "x";
    var snoozeEnabled = alarmData[0]["alarm"]["enabled"] ? "On" : "Off";
    snoozeSelect.locatePosition(0, snoozeMinArray.indexOf(snoozeDuration));
    snoozeSelect.locatePosition(1, snoozesArray.indexOf(snoozeLimit));
    snoozeSelect.locatePosition(2, enableArray.indexOf(snoozeEnabled));

    // Volume settings
    var volume = String(alarmData[0]["sound"]["volume"]);
    volumeSelect.locatePosition(0, volumeArray.indexOf(volume));

    // Audio settings
    var fileParts = alarmData[0]["sound"]["file"].split("/");
    var file = fileParts[fileParts.length - 1];
    audioSelect.locatePosition(0, audioArray.indexOf(file));

    // Timezone settings
    var timezoneMinute = Math.floor(alarmData[0]["timeOffset"] / 60) % 60;
    var timezoneHour = Math.floor(((alarmData[0]["timeOffset"] / 60) - timezoneMinute) / 60);
    if (timezoneHour >= 0) {
        timezoneHour = "+" + String(timezoneHour);
    }
    timezoneMinute = Math.abs(timezoneMinute);
    var timezone = String(timezoneHour) + ":" + String(timezoneMinute).padStart(2, '0');
    console.log(timezone);
    timezoneSelect.locatePosition(0, timezoneArray.indexOf(timezone));

    updateAlarmStatus(alarmData[0]["alarm"]["active"], alarmData[0]["snooze"]["active"], alarmData[0]["snooze"]["remaining"], alarmData[0]["snooze"]["enabled"]);
}
