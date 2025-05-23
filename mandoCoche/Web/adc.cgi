t <html><head><title>AD Input</title>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var formUpdate = new periodicObj("adc.cgx",1000);
t function plotADGraph_1() {
t  adVal = document.getElementById("ad_value_1").value;z
t }
t function periodicUpdateAdc() {
t  if(document.getElementById("adcChkBox").checked == true) {
t   updateMultiple(formUpdate, plotADGraph_1);
t   adc_elTime = setTimeout(periodicUpdateAdc, formUpdate.period);
t  }
t  else 
t   clearTimeout(adc_elTime);
t }
t </script></head>
i pg_header.inc
t <h2 align="center"><br>Parametros importantes</h2>
t <p><font size="2"></font></p>
t <form action="adc.cgi" method="post" name="adc">
t <input type="hidden" value="adc" name="pg">
t <table border=0 width=99%><font size="3">
t <tr style="background-color: #aaccff">
t <tr><td>Consumo</td>
t   <td align="center">
t <input type="text" readonly style="background-color: transparent; border: 0px"
c g 1  size="10" id="ad_value_1" value="0x%03X"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c g 2  size="10" id="ad_volts" value="%5.3f V"></td>
t <tr><td>Velocidad</td>
t   <td align="center">
t <input type="text" readonly style="background-color: transparent; border: 0px"
c g 4  size="10" id="ad_value_2" value="0x%03X"></td>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c g 5  size="10" id="ad_volts_2" value="%5.3f V"></td>
t <tr><td>Marcha</td>
t   <td align="center">
t <input type="text" readonly style="background-color: transparent; border: 0px"
c g 6  size="10" id="marcha" value="%d"></td>
t </font></table>
t <p align=center>
t <input type=button value="Refresh" onclick="updateMultiple(formUpdate, plotADGraph_1)">
t Periodic:<input type="checkbox" id="adcChkBox" onclick="periodicUpdateAdc()">
t </p></form>
i pg_footer.inc
. End of script must be closed with period