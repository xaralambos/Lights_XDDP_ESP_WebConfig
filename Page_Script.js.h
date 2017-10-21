


const char PAGE_microajax_js[] PROGMEM = R"=====(
//|
//function microAjax(B,A){this.bindFunction=function(E,D){return function(){return E.apply(D,[D])}};this.stateChange=function(D){if(this.request.readyState==4){this.callbackFunction(this.request.responseText)}};this.getRequest=function(){if(window.ActiveXObject){return new ActiveXObject("Microsoft.XMLHTTP")}else{if(window.XMLHttpRequest){return new XMLHttpRequest()}}return false};this.postBody=(arguments[2]||"");this.callbackFunction=A;this.url=B;this.request=this.getRequest();if(this.request){var C=this.request;C.onreadystatechange=this.bindFunction(this.stateChange,this);if(this.postBody!==""){C.open("POST",B,true);C.setRequestHeader("X-Requested-With","XMLHttpRequest");C.setRequestHeader("Content-type","application/x-www-form-urlencoded");C.setRequestHeader("Connection","close")}else{C.open("GET",B,true)}C.send(this.postBody)}};
function microAjax(url, orscF) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = orscF;
  xhttp.open("GET", url, true);
  xhttp.send();
}



function setValues(){ 
  if (this.readyState == 4 && this.status == 200) {
      
    var res = this.responseText;
    res.split(String.fromCharCode(10)).forEach(function(entry) {
      fields = entry.split("|");
  
      //console.log("fields[0]:" + fields[0] + "    fields[1]:" + fields[1] + "    fields[2]:" + fields[2]);
      if(fields[2] == "input"){
          document.getElementById(fields[0]).value = fields[1];
      }else if(fields[2] == "div"){
          //console.log(fields[2] + "  document.getElementById(  " + fields[0] +"  ).innerHTML  = " + fields[1]);
          document.getElementById(fields[0]).innerHTML  = fields[1];
      }else if(fields[2] == "chk"){
          document.getElementById(fields[0]).checked  = fields[1];
      }
    
    });

  }
}


function load(e, t, n) {
  if ("js" == t) {
    var a = document.createElement("script");
    a.src = e, a.type = "text/javascript", a.async = !1, a.onload = function() {n()}, document.getElementsByTagName("head")[0].appendChild(a)
  } else if ("css" == t) {
    var a = document.createElement("link");
    a.href = e, a.rel = "stylesheet", a.type = "text/css", a.async = !1, a.onload = function() {n()}, document.getElementsByTagName("head")[0].appendChild(a)
  }
}

function lightOnClick(elem) {
  var id = elem.id;
  //console.log('area element id = ' + id);
  
  if(elem.checked) {
    microAjax("/?" + id + "=ON", function (){} ); 
  }else{
    microAjax("/?" + id + "=OFF", function (){} ); 
  }
  
}


)=====";


void send_myFunctions_js(){
  String reply = ""; 
  
  reply += "//| \n";
  reply += "function lightOnClick(elem) { \n";
  reply += "  var id = elem.id; \n";
  reply += "  if(elem.checked) { \n";
  reply += "    microAjax(\"/api/others/?file=/api/?\" + id + \"=ON&host=" + WiFi.localIP().toString() + "&type=text\", function (){} );  \n";
  reply += "  }else{ \n";
  reply += "    microAjax(\"/api/others/?file=/api/?\" + id + \"=OFF&host=" + WiFi.localIP().toString() + "&type=text\", function (){} );  \n";
  reply += "  } \n";
  reply += "} \n";

  server.send ( 200, "text/plain", reply ); 
}


void send_othersMicroajax(){

  String reply = "";
  //reply += "loadOthers(); \n\n";
  
  reply += "function loadOthers(){ \n";
  reply += "  if(document.getElementById(\"ESPothers\").innerHTML == \"Gathering content from others...\"){ \n";
  reply += "    setTimeout(loadOthers ,100); \n";
  reply += "  }else{ \n";
  reply += "    microAjax(\"/api/others/?file=/api/content&host=192.168.0.202&type=html\", setValues ); \n";

  for(int i = 0; i < DevicesListSize && d[i].valid; i++){
    for(int j = 0; j < FileListSize && d[i].file_list[j].valid; j++){
      if(d[i].file_list[j].type == "html"){
        reply += "    microAjax(\"/api/others/?file=" + d[i].file_list[j].file_name + "&host=" + d[i].host.toString() + "&type=" + d[i].file_list[j].type + "\", setValues ); \n";         
      }
    }
  }
  
  reply += "  } \n";
  reply += "} \n\n";

  for(int i = 0; i < DevicesListSize && d[i].valid; i++){
    for(int j = 0; j < FileListSize && d[i].file_list[j].valid; j++){
      if(d[i].file_list[j].type == "js" || d[i].file_list[j].type == "css"){
        reply += "load(\"/api/others/?file=" + d[i].file_list[j].file_name + "&host=" + d[i].host.toString() + "&type=" + d[i].file_list[j].type + "\",\"" + d[i].file_list[j].type + "\", function(){} ); \n\n";
      }
    }
  } 
//  reply += "load(\"/api/others/?file=/api/myFunctions.js&host=192.168.0.204&type=js\",\"js\", function(){} ); \n";
//  reply += "load(\"/api/others/?file=/api/mystyle.css&host=192.168.0.204&type=css\",\"css\", function(){} ); \n";
  
  server.send ( 200, "text/plain", reply );
}


