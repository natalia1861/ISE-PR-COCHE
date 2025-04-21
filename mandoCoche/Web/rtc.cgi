t <html><head><title>RTC</title>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var formUpdate = new periodicObj("rtc.cgx", 500);
t function plotADGraph() {
t  hourVal = document.getElementById("rtc_time").value;
t  dateVal = document.getElementById("rtc_date").value;
t  document.getElementById("rtc_time").value = (hourVal);
t  document.getElementById("rtc_date").value = (dateVal);
t }
t function periodicUpdateAd() {
t  if(document.getElementById("adChkBox").checked == true) {
t   updateMultiple(formUpdate,plotADGraph);
t   ad_elTime = setTimeout(periodicUpdateAd, formUpdate.period);
t  }
t  else
t   clearTimeout(ad_elTime);
t }
t </script></head>
i pg_header.inc
t <h2 align="center"><br>RTC Module Control</h2>
t <p><font size="2">Esta pagina es para el reloj 1</font></p>
# formulario para mandar datos
t <form action="rtc.cgi" method="post" name="rtc">
t <input type="hidden" value="rtc" name="pg">
t <table border=0 width=99%><font size="3">
t <tr style="background-color: #aaccff">
t  <th width=30%>Item</th>
t  <th width=30%>Value</th>
t <tr><td><img src="pabb.gif">Time:</td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 5px"
c h 1  size="20" id="rtc_time" value="%s"></td>
t <tr><td><img src="pabb.gif">Date:</td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 5px"
c h 2  size="20" id="rtc_date" value="%s"></td>
t </font></table>
t <p align=center>
t <input type=button value="Refresh" onclick="updateMultiple(formUpdate,plotADGraph)">
t Periodic:<input type="checkbox" id="adChkBox" onclick="periodicUpdateAd()">
t </p></form>
i pg_footer.inc
. End of script must be closed with period