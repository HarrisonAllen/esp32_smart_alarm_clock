var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);

var max_volume = 21;
var min_volume = 0;

function onload(event) {
    initWebSocket();
}

function getData(){
    websocket.send("getData");
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

function updateAlarmLabel(element) {
    var newLabel = document.getElementById(element.id).value;
    console.log("l" + newLabel);
    websocket.send("l" + newLabel);
}

function updateAlarmEnabled(element) {
    var alarmEnabled = document.getElementById(element.id).checked;
    console.log("e" + alarmEnabled);
    websocket.send("e" + alarmEnabled);
}

function updateAlarmTime(element) {
    var alarmTime = document.getElementById(element.id).value;
    console.log("t" + alarmTime);
    websocket.send("t" + alarmTime);
}

function updateAlarmRepeats(element) {
    var alarmRepeats = document.getElementById(element.id).checked;
    console.log("r" + alarmRepeats);
    websocket.send("r" + alarmRepeats);
}

function updateSnoozeEnabled(element) {
    var snoozeEnabled = document.getElementById(element.id).checked;
    console.log("E" + snoozeEnabled);
    websocket.send("E" + snoozeEnabled);
}

function updateSnoozeDuration(element) {
    var snoozeDuration = document.getElementById(element.id).value;
    console.log("d" + snoozeDuration);
    websocket.send("d" + snoozeDuration.toString());
}

function updateSnoozeLimit(element) {
    var snoozeLimit = document.getElementById(element.id).value;
    console.log("L" + snoozeLimit);
    websocket.send("L" + snoozeLimit.toString());
}

function updateVolume(element) {
    var volume = document.getElementById(element.id).value;
    document.getElementById("volume").textContent = volume;

    console.log("v" + volume);
    websocket.send("v" + volume.toString());
}

function updateRampEnabled(element) {
    var rampEnabled = document.getElementById(element.id).checked;
    console.log("R" + rampEnabled);
    websocket.send("R" + rampEnabled);
}

function updateSoundFile(element) {
    var soundFile = document.getElementById(element.id).value;
    console.log("f" + soundFile);
    websocket.send("f" + soundFile);
}

function snooze(element) {
    console.log("s");
    websocket.send("s");
}

function stop(element) {
    console.log("S");
    websocket.send("S");
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
    var alarmData = JSON.parse(event.data);
    // var keys = Object.keys(alarmData);

    document.getElementById("alarmLabel").value = alarmData["alarm"]["label"];
    document.getElementById("alarmEnabled").checked = alarmData["alarm"]["alarm"]["enabled"] === "true";
    document.getElementById("alarmTime").value = alarmData["alarm"]["alarm"]["time"];
    document.getElementById("alarmRepeats").checked = alarmData["alarm"]["alarm"]["repeat"] === "true";

    document.getElementById("snoozeEnabled").checked = alarmData["alarm"]["snooze"]["enabled"] === "true";
    document.getElementById("snoozeDuration").value = alarmData["alarm"]["snooze"]["duration"];
    document.getElementById("snoozeLimit").value = alarmData["alarm"]["snooze"]["limit"];

    document.getElementById("volumeSlider").value = alarmData["alarm"]["sound"]["volume"];
    document.getElementById("volume").textContent = alarmData["alarm"]["sound"]["volume"];
    document.getElementById("rampEnabled").checked = alarmData["alarm"]["sound"]["ramp"] === "true";
    document.getElementById("soundFileSelect").value = alarmData["alarm"]["sound"]["file"];

    updateAlarmVolumeIcon(alarmData["alarm"]["sound"]["volume"]);
    updateAlarmStatus(alarmData["alarm"]["alarm"]["active"] === "true", alarmData["alarm"]["snooze"]["active"] === "true", alarmData["alarm"]["snooze"]["remaining"], alarmData["alarm"]["snooze"]["enabled"] === "true");

    // for (var i = 0; i < keys.length; i++){
    //     var key = keys[i];
    //     document.getElementById(key).innerHTML = alarmData[key];
    //     document.getElementById("slider"+ (i+1).toString()).value = alarmData[key];
    // }
}
