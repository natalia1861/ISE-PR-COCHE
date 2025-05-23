t <html><head><title>CONTROL </title>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var formUpdate = new periodicObj("control.cgx",1000);
t var distanciaPrevio = ""; // Variable para almacenar el valor previo 
t var consumoPrevio = ""; // Variable para almacenar el valor previo 
t function plotGraph() {
# Obtenemos los valores
t  distancia = document.getElementById("Distancia").value;
t  consumo = document.getElementById("Consumo").value;
t  marcha = document.getElementById("Marcha").value;
t  direccion = document.getElementById("Direccion").value;
t  hora = document.getElementById("RTC_HORA").value;
t  fecha = document.getElementById("RTC_FECHA").value;
t  if (distancia != distanciaPrevio) {
t    distanciaPrevio = distancia;
t    addEventToHistory(" Distancia: " + distancia);
t  }
t  if (consumo != consumoPrevio) {
t    consumoPrevio = consumo;
t    addEventToHistory(" Consumo: " + consumo);
t  }
t }
t function sendUpdate() {
t    distancia = document.getElementById("Distancia");
t    consumo = document.getElementById("Consumo");
t    marcha = document.getElementById("Marcha");
t    Direccion = document.getElementById("Direccion");
t }
t function periodicUpdate() {
t  if(document.getElementById("ControlChkBox").checked == true) {
t   updateMultiple(formUpdate,plotGraph);
t   contr_elTime = setTimeout(periodicUpdate, formUpdate.period);
t  }
t  else
t   clearTimeout(contr_elTime);
t }
t function addEventToHistory(event) {
t  var historyElement = document.getElementById("history");
t  var newEvent = document.createElement("li");
t  newEvent.textContent = event;
t  newEvent.style.color = "black"; // Cambia el color del texto a blanco
t  newEvent.style.fontSize = "12px"; // Aumenta el tamaño del texto
t  historyElement.appendChild(newEvent);
t }
t </script></head>
i pg_header.inc
t <h2 align=center><br>PARAMETROS PRINCIPALES </h2>
t <form action=control.cgi method=post name=cgi>
t <input type=hidden value="control" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#fffacd>
t  <th width=40%>VARIABLE</th>
t  <th width=60%>VALOR</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t <tr><td>Distancia: </td>
c g 1 <td><input type=text name=dist id="Distancia" value="%s" size=20 maxlength=20"></td></tr>
t <tr><td> Consumo: </TD>
c g 2 <td><input type=text name=consumo id="Consumo" value="%s" size=20 maxlength=20"></td></tr>
t <tr><td>Marcha: </td>
c g 3 <td><input type=text name=marcha id="Marcha" value="%s" size=20 maxlength=20"></td></tr>
t <tr><td>Direccion: </TD>
c g 4 <td><input type=text name=direccion id="Direccion" value="%s" size=20 maxlength=20"></td></tr>
t <tr><td>Hora del sistema:</td>
c g 5 <td><input type=text name=rtc1 id="RTC_HORA" value="%s" size=20 maxlength=20"></td></tr>
t <tr><td>Fecha del sistema:</TD>
c g 6 <td><input type=text name=rtc2 id="RTC_FECHA" value="%s"size=20 maxlength=20"></td></tr>
t </font></table>
t <p align=center>
t Activar:<input type="checkbox" id="ControlChkBox" onclick="periodicUpdate()">
t </p></form>
t <h3>HISTORIAL</h3>
t <ul id="history"></ul>
i pg_footer.inc
. End of script must be closed with period.