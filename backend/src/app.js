const express = require('express')
const mongoose = require('mongoose');

// npm start
// docker run -d -p 27017:27017 --name m1 mongo

mongoose.connect("mongodb://127.0.0.1:27017/",{
        useCreateIndex:true,
        useNewUrlParser: true,
        useUnifiedTopology: true}).then(()=> {
          console.log('Database Successfully Connected')},
          error =>{
            console.log(error)}
        );

const app = express()
const port = 3000
  
app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`)
})

const SensorType = Object.freeze({
  Temperature: "Temperature"
});

const SensorSchema = new mongoose.Schema({
  id: Number,
  type: Object.values(SensorType)
});

Object.assign(SensorSchema.statics, { SensorType });

const DataEntrySchema = new mongoose.Schema({
  datetime: String,
  sensor: SensorSchema,
  value: Number
});

const DataEntry = mongoose.model('DataEntry', DataEntrySchema);
const Sensor = mongoose.model('Sensor', SensorSchema);

const sensor1 = new Sensor( { id: 528201, type: SensorType.Temperature } );
sensor1.save();

const entry1 = new DataEntry(
  {
    datetime: Date.now(),
    sensor: sensor1,
    value: 2502
  }
);
entry1.save();

app.get('/sensors', (req, res) => {
  Sensor.find({}, function(err, sensors) {
    res.send(sensors);
  });
})

app.get('/entries', (req, res) => {
  DataEntry.find({}, function(err, entries) {
    res.send(entries);
  });
})
