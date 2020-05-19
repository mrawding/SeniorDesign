const cool = require('cool-ascii-faces')
const express = require('express')
const plotly = require('plotly')('mrawding1','B4Yi1l3Ydelth3oLyfkp')
const fs = require('fs')
const http = require('http')
const path = require('path')
const PORT = process.env.PORT || 5000
const { Pool } = require('pg')
const pool = new Pool({ connectionString: process.env.DATABASE_URL,ssl: true})
const bodyParser = require('body-parser')
const mqtt = require('mqtt'), url = require('url');

const mqtt_url = url.parse(process.env.CLOUDMQTT_URL || 'mqtt://localhost:1883');
const auth = (mqtt_url.auth || ':').split(':');

const app = express();
const server = http.createServer(app);
const io = require('socket.io').listen(server)
var mqtt_client = mqtt.connect(mqtt_url);


  app.use(express.static(path.join(__dirname, 'public')))
  app.use(bodyParser.json())
  app.set('views', path.join(__dirname, 'views'))
  app.set('view engine', 'ejs')
  app.get('/', (req, res) => res.render('pages/index'));
  app.get('/location', (req, res) => res.render('pages/geo'));
  app.get('/db', async (req,res) =>
	  
	  {		

			 res.render('pages/db');


	  	
  });

 app.get('/download',async(req,res) =>
	 {
	 
		text = 'SELECT * from BLUSENSE';
		pool.query(text,(err,resp) => 
			{
			if(err)
			{
				console.log("Query Error");
				console.log(err)
			}
			else{
				try
				{
					const data = fs.writeFileSync('./raw.txt',resp)
					console.log("Writing to file");
						
					
				}
				catch(err)
				{
					console.error(err);

				}

			}
		});

		 res.render('pages/db');

	 } );

	  	
  app.listen(PORT, function () {
	              console.log('listening');
	    });

mqtt_client.on('connect', function(){     
	var data = {};
	var ph = 0;                                                                                                             var temp =0;
	var tds = 0;
	var press = 0;
	var cond = 0;
	var diso = 0;
	console.log('mqtt client connected');
	mqtt_client.subscribe('sensors/ph');
	mqtt_client.subscribe('sensors/pressure');
	mqtt_client.subscribe('sensors/tds');
	mqtt_client.subscribe('sensors/temperature');
	mqtt_client.subscribe('sensors/conductivity');
	mqtt_client.subscribe('sensors/dO');
	mqtt_client.on('message', function(topic,message,packet)
	
	{
			
			var result = message.toString();
	//		io.emit('mssg',result);;
		if(topic == 'sensors/ph')
		{
			data.ph = result;
			ph =1;
			console.log('ph: ' + result + 'flag' + ph);
		}
		else if(topic == 'sensors/pressure')
		{
			data.pressure = result;
			press = 1;
			console.log('pressure: ' + result );
		}
		else if(topic == 'sensors/tds')
		{
			data.tds = result;
			tds = 1;
			console.log('tds: ' + result);
		}
		else if(topic == 'sensors/temperature')
		{
			data.temp = result;
			temp = 1;
			console.log('temp: ' + result);
		}
		else if(topic == 'sensors/conductivity')
		{
			data.cond = result;
			cond = 1;
			console.log('cond: ' + result);
		}
		else if(topic == 'sensors/dO')
		{
			data.diso = result;
			diso =1;
			console.log("D02: " + result);
		}
		else
		{
			console.log("Invalid Topic");
		}
			


if(tds == 1 && press ==1 && temp ==1 && ph==1 && cond == 1 && diso == 1)
			{

			var date = new Date();
			var mins = date.toTimeString();
			var time = date.toDateString();
			time = time + ' '+ mins;
				            
			var text = 'INSERT INTO blusense(time,pH,temperature,pressure,dO2,conduct,TDS) VALUES($1,$2,$3,$4,$5,$6,$7) RETURNING *;';                                          
		
	
			var values = [time,data.ph,data.temp,data.pressure,data.dO2,data.cond,data.tds];

			
			pool.query(text,values,(err,resp) => {

			
			if(err)

			
			{

			
				console.log(err);

	
			}


			else{

			
				console.log(resp.rows[0]);
				tds = press = temp = ph = cond = diso = 0;
				graphData();

				
			}

			
			});
			}
	
	else{
		console.log('mqtt message received!');
	}
});
});

function graphData() {
	var text = "SELECT time,pH,do2,temperature,conduct,tds,pressure FROM blusense;"
  	pool.query(text,(err,resp) => {
	if(err)
	{
 	 console.log("pool query error");
 	 console.log(err);
	}
	else{
//res.send(resp.rows);
 	 var ph = []
	  var times = []
 	 var tds = []
 	 var diso = []
 	 var temp = []
 	 var cond = []
 	 var press = []
  	for(i = 0; i < resp.rows.length; i++)
	  {
   	 times.push(resp.rows[i].time);
   	 ph.push(resp.rows[i].ph);
  	  diso.push(resp.rows[i].do2);
   	 temp.push(resp.rows[i].temperature);
   	 cond.push(resp.rows[i].conduct);
   	 press.push(resp.rows[i].pressure);
  	  tds.push(resp.rows[i].tds);
	
	  }
	  pHlayout = {
   	 title: "pH",
  	  xaxis: {
     	 title: "Date",
     	 titlefont:{
       	 family: "Courier New, monospace",
       	 size: 18,
       	 color: "skyblue"
     	 }
   	 },
	    yaxis: {
   	   title: "pH",
  	    titlefont:{
                                        	    family: "Courier New, monospace",
                                	            size: 18,
                        	                   color: "skyblue"
                	                   }
        	                   }
 	 };

  	 templayout = {
                             title: "Temperature ",
                             xaxis: {
                                     title: "DATE",
                                     titlefont:{
                                             family: "Courier New, monospace",
                                             size: 18,
                                             color: "skyblue"
                                    }
                          },
                             yaxis: {
                                     title: "Degrees C",
                                      titlefont:{
                                             family: "Courier New, monospace",
                                           size: 18,
                                             color: "skyblue"
                                  }
                          }
                  };
 	disolayout = {
                          title: "Dissolved Oxygen",
                          xaxis: {
                                  title: "DATE",
                                  titlefont:{
                                          family: "Courier New, monospace",
                                          size: 18,
                                          color: "skyblue"
                                  }
                          },
                          yaxis: {
                                  tile: "mg/L",
                                  titlefont:{
                                          family: "Courier New, monospace",
                                          size: 18,
                                          color: "skyblue"
                                  }
                          }
                  };

	 condlayout = {
                          title: "Fuck",
                          xaxis: {
                                  title: "DATE",
                                  titlefont:{
                                          family: "Courier New, monospace",
                                          size: 18,
                                          color: "skyblue"
                                  }
                          },
                          yaxis: {
                                  title: "ms/cm",
                                  titlefont:{
                                          family: "Courier New, monospace",
                                          size: 18,
                                          color: "skyblue"
                                  }
                          }
                  };

	 presslayout = {
                          title: "Pressure",
                          xaxis: {
                                  title: "DATE",
                                  titlefont:{
                                          family: "Courier New, monospace",
                                          size: 18,
                                          color: "skyblue"
                                  }
                          },
                          yaxis: {
                                  title: "Pressure (atm)",
                                  titlefont:{
                                          family: "Courier New, monospace",
                                          size: 18,
                                          color: "skyblue"
                                  }
                          }
                  };
	 tdslayout = {
                          title: "Total Dissolved Solids",
                          xaxis: {
                                  title: "DATE",
                                  titlefont:{
                                          family: "Courier New, monospace",
                                          size: 18,
                                          color: "skyblue"
                                  }
                          },
                          yaxis: {
                                  title: "PPM",
                                  titlefont:{
                                          family: "Courier New, monospace",
                                          size: 18,
                                          color: "skyblue"
                                  }
                          }
                  };

	phdata = { x: times, y: ph,type: "scatter" };
	tdsdata = { x: times, y: tds,type: "scatter" };
	conddata = { x: times, y: cond,type: "scatter" };
	pressdata = { x: times, y: press,type: "scatter" };
	tempdata = { x: times, y: temp,type: "scatter" };
	disodata = { x: times, y: diso,type: "scatter" };
	var graphOptions1 = {layout: pHlayout, filename: "date-axes", fileopt: "overwrite"};
	var graphOptions2 = {layout: condlayout, filename: "cond", fileopt: "overwrite"};
	var graphOptions3 = {layout: templayout, filename: "temp", fileopt: "overwrite"};
	var graphOptions4 = {layout: tdslayout, filename: "tds", fileopt: "overwrite"};
	var graphOptions5 = {layout: presslayout, filename: "press", fileopt: "overwrite"};
	var graphOptions6 = {layout: disolayout, filename: "diso", fileopt: "overwrite"};
	plotly.plot(phdata, graphOptions1, function (err, msg) {
     		console.log("ph graphed");
     		console.log(err)});
	plotly.plot(conddata, graphOptions2, function (err, msg) {
                             console.log("cond");});
	plotly.plot(tempdata, graphOptions3, function (err, msg) {
                             console.log("temp");});
	plotly.plot(tdsdata, graphOptions4, function (err, msg) {
                             console.log("tds");});
	plotly.plot(pressdata, graphOptions5, function (err, msg) {
                             console.log("Pressure");});
	plotly.plot(disodata, graphOptions6, function (err, msg) {
                             console.log("Dissolved");});
	}
	});
}
