var chartT = new Highcharts.Chart({
  chart:{ renderTo : 'chart-temperature' },
  title: { 
      text: 'MAX30102 Heart Rate', 
      style: {
          color: 'rgb(255, 241, 220)',
          font: 'Poppins'
      }

  },
  series: [{
    showInLegend: false,
    data: []
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    },
    series: { color: '#059e8a' }
  },
  xAxis: { type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { 
        text: 'Beats per Minute (BPM)',
        style: {
          color: 'rgb(255, 241, 220)',
          font: 'Poppins'
        } 
      }
    //title: { text: 'Temperature (Fahrenheit)' }
  },
  credits: { enabled: false }
});
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("heartrate").innerHTML = this.responseText;
      var x = (new Date()).getTime(),
          y = parseFloat(this.responseText);
      //console.log(this.responseText);
      if(chartT.series[0].data.length > 40) {
        chartT.series[0].addPoint([x, y], true, true, true);
      } else {
        chartT.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/heartrate", true);
  xhttp.send();
}, 5000);
  
var chartH = new Highcharts.Chart({
  chart:{ renderTo:'chart-humidity' },
  title: { 
      text: 'MAX30102 Oxygen Saturation (SpO2)',
      style: {
          color: 'rgb(255, 241, 220)',
          font: 'Poppins'
      } 
  },
  series: [{
    showInLegend: false,
    data: []
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    }
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { 
        text: 'Percentage (%)',
        style: {
          color: 'rgb(255, 241, 220)',
          font: 'Poppins'
        }
      }
  },
  credits: { enabled: false }
});
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("oxygensaturation").innerHTML = this.responseText;
      var x = (new Date()).getTime(),
          y = parseFloat(this.responseText);
      //console.log(this.responseText);
      if(chartH.series[0].data.length > 40) {
        chartH.series[0].addPoint([x, y], true, true, true);
      } else {
        chartH.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/SpO2", true);
  xhttp.send();
}, 5000);
  
var chartP = new Highcharts.Chart({
  chart:{ renderTo:'chart-pressure' },
  title: { 
      text: 'MAX30102 Temperature',
      style: {
      color: 'rgb(255, 241, 220)',
      font: 'Poppins'
      } 
  },
  series: [{
    showInLegend: false,
    data: []
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    },
    series: { color: '#18009c' }
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { 
        text: 'Celsius (&deg;C)',
        style: {
          color: 'rgb(255, 241, 220)',
          font: 'Poppins'
        } 
      }
  },
  credits: { enabled: false }
});
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
      var x = (new Date()).getTime(),
          y = parseFloat(this.responseText);
      //console.log(this.responseText);
      if(chartP.series[0].data.length > 40) {
        chartP.series[0].addPoint([x, y], true, true, true);
      } else {
        chartP.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 5000);

//   Digital Clock Function
function updateClock(){
    var now = new Date();
    var dname = now.getDay(),
        mo = now.getMonth(),
        dnum = now.getDate(),
        yr = now.getFullYear(),
        hou = now.getHours(),
        min = now.getMinutes(),
        sec = now.getSeconds(),
        pe = "AM";

    if(hou == 0){
        hou = 12;
    }
    if(hou > 12){
        hou = hou - 12;
        pe = "PM";
    }

    Number.prototype.pad = function(digits){
        for(var n = this.toString(); n.length < digits; n = 0 + n);
        return n;
    }

    var months = ["January", "February", "March", "April", "May", "June", "July", "Augest", "September", "October", "November", "December"];
    var week = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
    var ids = ["dayname", "month", "daynum", "year", "hour", "minutes", "seconds", "period"];
    var values = [week[dname], months[mo], dnum.pad(2), yr, hou.pad(2), min.pad(2), sec.pad(2), pe];
    
    for(var i = 0; i < ids.length; i++)
    document.getElementById(ids[i]).firstChild.nodeValue = values[i];
}

function initClock(){
    window.setInterval("updateClock()", 1);
}