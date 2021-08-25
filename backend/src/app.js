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

app.get('/', (req, res) => {
    res.send('Hello World!')
  })
  
app.listen(port, () => {
console.log(`Example app listening at http://localhost:${port}`)
})
