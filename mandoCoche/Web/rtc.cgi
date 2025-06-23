t <html><head><title>MEMORIA FLASH</title></head>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var formUpdate = new periodicObj("rtc.cgx", 1000);
t function plotRTCGraph() {
t  hora = document.getElementById("RTC_HORA").value;
t  fecha = document.getElementById("RTC_FECHA").value;
t }
t function sendUpdate() {
t  fecha = document.getElementById("RTC_FECHA").value;
t  hora = document.getElementById("RTC_HORA").value;
t }
t function periodicUpdateRTC() {
t  if(document.getElementById("RTCChkBox").checked == true) {
t   updateMultiple(formUpdate,plotRTCGraph);
t   rtc_elTime = setTimeout(periodicUpdateRTC, formUpdate.period);
t  }
t  else
t   clearTimeout(rtc_elTime);
t }
t </script></head>
i pg_header.inc
t <h2 align=center><br>MEMORIA FLASH </h2>
t <form action=rtc.cgi method=post name=cgi>
t <input type=hidden value="flash" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#ffeb99>
t  <th width=50%>HORA</th>
t  <th width=50%>CONSUMO</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t </tr>
t <tr>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c i 0  size="10" id="fecha0" value="%s"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c z 0  size="10" id="consumo0" value="%s"></td>
t </tr>
t <tr>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c i 1  size="10" id="fecha1" value="%s"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c z 1  size="10" id="consumo1" value="%s"></td>
t </tr>
t <tr>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c i 2  size="10" id="fecha2" value="%s"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c z 2  size="10" id="consumo2" value="%s"></td>
t </tr>
t <tr>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c i 3  size="10" id="fecha3" value="%s"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c z 3  size="10" id="consumo3" value="%s"></td>
t </tr>
t <tr>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c i 4  size="10" id="fecha4" value="%s"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c z 4  size="10" id="consumo4" value="%s"></td>
t </tr>
t <tr>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c i 5  size="10" id="fecha5" value="%s"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c z 5  size="10" id="consumo5" value="%s"></td>
t </tr>
t <tr>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c i 6  size="10" id="fecha6" value="%s"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c z 6  size="10" id="consumo6" value="%s"></td>
t </tr>
t <tr>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c i 7  size="10" id="fecha7" value="%s"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c z 7  size="10" id="consumo7" value="%s"></td>
t </tr>
t <tr>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c i 8  size="10" id="fecha8" value="%s"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c z 8  size="10" id="consumo8" value="%s"></td>
t </tr>
t <tr>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c i 9  size="10" id="fecha9" value="%s"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c z 9  size="10" id="consumo9" value="%s"></td>
t  </td>
t </tr>
t </font></table>
# Here begin button definitions
t <p align=center>
t Activar:<input type="checkbox" id="RTCChkBox" onclick="periodicUpdateRTC()">
t </p></form>
i pg_footer.inc
. End of script must be closed with period.
