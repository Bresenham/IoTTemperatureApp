const cors = require('cors');
const express = require('express');
const bodyParser = require('body-parser');
const mongoose = require('mongoose');
const path = require('path');
const fs = require('fs');

const fill_default_data = process.env.TEST;

const app = express();
app.use(bodyParser.json())

const port = 3000

// Set up CORS
app.use(cors({
    origin: true, // "true" will copy the domain of the request back
                  // to the reply. If you need more control than this
                  // use a function.

    credentials: true, // This MUST be "true" if your endpoint is
                       // authenticated via either a session cookie
                       // or Authorization header. Otherwise the
                       // browser will block the response.

    methods: 'POST,GET,PUT,OPTIONS,DELETE' // Make sure you're not blocking
                                           // pre-flight OPTIONS requests
}));

const sensorRouter = require('./routes/sensors');
const dataRouter = require('./routes/data');
const DataEntry = require('./models/dataEntry');
const Sensor = require('./models/sensor');

app.use('/sensor', sensorRouter);
app.use('/data', dataRouter);

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`)
})

mongoose.connect("mongodb://127.0.0.1:27017/",{
        useCreateIndex:true,
        useNewUrlParser: true,
        useUnifiedTopology: true}
).then(() => {
    console.log('Database Successfully Connected')
    if(fill_default_data) {

      Sensor.create(
        JSON.parse(fs.readFileSync(path.resolve(__dirname, 'test_sensors.json'), 'utf8'))
      );

      DataEntry.create(
        JSON.parse(fs.readFileSync(path.resolve(__dirname, 'test_data_entries.json'), 'utf8'))
      );

    }
  }, error => {
    console.log(error)
  }
);

