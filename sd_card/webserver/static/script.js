var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);

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

// function updateSliderPWM(element) {
//     var sliderNumber = element.id.charAt(element.id.length-1);
//     var sliderValue = document.getElementById(element.id).value;
//     document.getElementById("sliderValue"+sliderNumber).innerHTML = sliderValue;
//     console.log(sliderValue);
//     websocket.send(sliderNumber+"s"+sliderValue.toString());
// }

function updateAlarmTime(element) {
    var alarmTime = document.getElementById(element.id).value;
    console.log("a" + alarmTime);
    websocket.send("a" + alarmTime);
}

function updateAlarmEnabled(element) {
    var alarmEnabled = document.getElementById(element.id).checked;
    console.log("e" + alarmEnabled);
    websocket.send("e" + alarmEnabled);
}

function snooze(element) {
    console.log("s");
    websocket.send("s");
}

function onMessage(event) {
    console.log(event.data);
    var alarmData = JSON.parse(event.data);
    // var keys = Object.keys(alarmData);

    document.getElementById("currentTime").innerHTML = alarmData["currentTime"];
    document.getElementById("alarmTime").value = alarmData["alarm"]["alarm"]["time"];
    document.getElementById("alarmEnabled").checked = alarmData["alarm"]["alarm"]["enabled"] === "true";
    document.getElementById("snoozeButton").disabled = alarmData["alarm"]["alarm"]["active"] === "false";

    // for (var i = 0; i < keys.length; i++){
    //     var key = keys[i];
    //     document.getElementById(key).innerHTML = alarmData[key];
    //     document.getElementById("slider"+ (i+1).toString()).value = alarmData[key];
    // }
}
