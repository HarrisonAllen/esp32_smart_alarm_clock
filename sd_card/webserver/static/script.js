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

function updateVolume(element) {
    var volume = document.getElementById(element.id).value;
    document.getElementById("volume").textContent = volume;
    updateAlarmVolumeIcon(volume);
}

function save() {
    alarmData[0]["timeOffset"] = Number(document.getElementById("timeOffset").value);
    alarmData[0]["label"] = document.getElementById("alarmLabel").value;
    alarmData[0]["hidden"] = false;
    alarmData[0]["alarm"]["enabled"] = document.getElementById("alarmEnabled").checked;
    var alarmTime = document.getElementById("alarmTime").value.split(':');
    var hours = Number(alarmTime[0]);
    var minutes = Number(alarmTime[1]);
    alarmData[0]["alarm"]["hour"] = hours;
    alarmData[0]["alarm"]["minute"] = minutes;
    alarmData[0]["alarm"]["currentHour"] = hours;
    alarmData[0]["alarm"]["currentMinute"] = minutes;
    alarmData[0]["alarm"]["repeat"] = document.getElementById("alarmRepeats").checked;
    alarmData[0]["snooze"]["enabled"] = document.getElementById("snoozeEnabled").checked;
    alarmData[0]["snooze"]["duration"] = Number(document.getElementById("snoozeDuration").value);
    alarmData[0]["snooze"]["limit"] = Number(document.getElementById("snoozeLimit").value);
    alarmData[0]["sound"]["volume"] = Number(document.getElementById("volumeSlider").value);
    alarmData[0]["sound"]["ramp"] = document.getElementById("rampEnabled").checked;
    alarmData[0]["sound"]["file"] = document.getElementById("soundFileSelect").value;
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
    var status = document.getElementById("status");
    var settings = document.getElementById("settings");
    var snoozeButtonContainer = document.getElementById("snoozeButtonContainer");
    var stopButtonContainer = document.getElementById("stopButtonContainer");
    var alarmName = document.getElementById("alarmLabel").value;
    if (alarmActive) {
        status.hidden = false;
        settings.hidden = true;
        alarmStatus.textContent = "🚨 " + alarmName + " 🚨";
        stopButtonContainer.hidden = false;
    } else if (snoozeActive) {
        status.hidden = false;
        settings.hidden = true;
        alarmStatus.textContent = "😴 " + alarmName + " (" + snoozesRemaining + " left)";
        stopButtonContainer.hidden = false;
    } else {
        status.hidden = true;
        settings.hidden = false;
    }
    snoozeButtonContainer.hidden = !(alarmActive && snoozeEnabled && !snoozeActive && (snoozesRemaining > 0));
}

function updateAlarmVolumeIcon(volume) {
    var volumeIcon = document.getElementById("volumeIcon")
    if (volume === min_volume) {
        volumeIcon.textContent = "🔈";
    } else if (volume < (max_volume / 2)) {
        volumeIcon.textContent = "🔉";
    } else {
        volumeIcon.textContent = "🔊";
    }
}

function onMessage(event) {
    console.log(event.data);
    alarmData = JSON.parse(event.data); 

    document.getElementById("timeOffset").value = alarmData[0]["timeOffset"];

    document.getElementById("alarmLabel").value = alarmData[0]["label"];
    document.getElementById("alarmEnabled").checked = alarmData[0]["alarm"]["enabled"];
    document.getElementById("alarmTime").value = String(alarmData[0]["alarm"]["hour"]).padStart(2, '0') + ":" + String(alarmData[0]["alarm"]["minute"]).padStart(2, '0');
    document.getElementById("alarmRepeats").checked = alarmData[0]["alarm"]["repeat"];

    document.getElementById("snoozeEnabled").checked = alarmData[0]["snooze"]["enabled"];
    document.getElementById("snoozeDuration").value = alarmData[0]["snooze"]["duration"];
    document.getElementById("snoozeLimit").value = alarmData[0]["snooze"]["limit"];

    document.getElementById("volumeSlider").value = alarmData[0]["sound"]["volume"];
    document.getElementById("volume").textContent = alarmData[0]["sound"]["volume"];
    document.getElementById("rampEnabled").checked = alarmData[0]["sound"]["ramp"];
    document.getElementById("soundFileSelect").value = alarmData[0]["sound"]["file"];

    updateAlarmVolumeIcon(alarmData[0]["sound"]["volume"]);
    updateAlarmStatus(alarmData[0]["alarm"]["active"], alarmData[0]["snooze"]["active"], alarmData[0]["snooze"]["remaining"], alarmData[0]["snooze"]["enabled"]);
}
