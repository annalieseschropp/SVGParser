//Annaliese Schropp
//1013472
//CIS 2750 A4
'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');
const bodyParser = require('body-parser');
const directoryPath = path.join(__dirname, '/uploads');
var listOfFiles = new Array();
var rectInfo = new Array();
var circInfo = new Array();
var pathInfo = new Array();
var groupInfo = new Array();
var svgInfo = new Array();
var login = new Array();
var dataFILE;
var dataCHANGE;
var dataDOWNLOAD;
var addedSuccessfully;
var valid;

// Database
const mysql = require('mysql2/promise');
let connection;

//C library functions
var lib = ffi.Library('./libsvgparse', {
  'getTableInfo': [ 'string', [ 'string' ] ],
  'getCircInfo': [ 'string', [ 'string' ] ],
  'getRectInfo': [ 'string', [ 'string' ] ],
  'getPathInfo': [ 'string', [ 'string' ] ],
  'getGroupInfo': [ 'string', [ 'string' ] ],
  'getSVGInfo': [ 'string', [ 'string' ] ],
  'getSVGAttr': [ 'string', [ 'string' ] ],
  'getDesc': [ 'string', [ 'string' ] ],
  'getTitle': [ 'string', [ 'string' ] ],
  'setTitle': [ 'void', [ 'string', 'string' ] ],
  'setDescr': [ 'void', [ 'string', 'string' ] ],
  'newSVGFile': [ 'bool', [ 'string', 'string', 'string' ] ],
  'addNewRect': [ 'bool', [ 'string', 'string', 'string', 'string', 'string', 'string' ] ],
  'addNewCirc': [ 'bool', [ 'string', 'string', 'string', 'string', 'string' ] ],
  'getRectAttr': [ 'string', [ 'string', 'int' ] ],
  'getCircAttr': [ 'string', [ 'string', 'int' ] ],
  'getPathAttr': [ 'string', [ 'string', 'int' ] ],
  'getGroupAttr': [ 'string', [ 'string', 'int' ] ],
  'addNewAttribute': [ 'bool', [ 'string', 'string', 'int', 'string', 'string' ] ],
  'validateFile': [ 'bool', [ 'string' ] ]
});

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));
// parse application/x-www-form-urlencoded
app.use(bodyParser.urlencoded({ extended: false }))
// parse application/json
app.use(bodyParser.json())

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
    valid = lib.validateFile(uploadFile.name);
    if (valid == true) {
      console.log("Valid file!");
      valid = true;
      listOfFiles.push(uploadFile.name);
    } else {
      console.log("Invalid file!");
      valid = false;
    }
    console.log(listOfFiles);
  });
});

// If file is invalid, move it to the invalid folder
app.post('/moveFile', function(req, res) {
  let invalidFile = req.files.uploadFile;
  let oldPath = "uploads/"+invalidFile.name;

  fs.unlinkSync(oldPath);

  // Use the mv() method to place the file somewhere on your server
  invalidFile.mv('invalid/' + invalidFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }
    res.redirect('/');
    const index = listOfFiles.indexOf(invalidFile.name);
    if (index > -1) {
      listOfFiles.splice(index, 1);
    }
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

// Checks if something is valid
app.get('/valid', function(req, res) {
  res.send(valid);
});

// Reads all the files 
fs.readdir(directoryPath, function (err, files) {
    if (err) {
        return console.log('Unable to scan directory: ' + err);
    } 
    //listing all files
    files.forEach(function (file) {
        listOfFiles.push(file);
    });
});

// Gets all the files, also checks if they are still valid
app.get('/files', function(req, res) {
  var check;
  for (var i = 0; i < listOfFiles.length; i++) {
    check = lib.validateFile(listOfFiles[i]);
    if (check == false) {
      console.log(listOfFiles[i]+" is not valid. Removed.");
      let oldPath = "uploads/"+listOfFiles[i];
      fs.unlinkSync(oldPath);
      const index = listOfFiles.indexOf(listOfFiles[i]);
      if (index > -1) {
        listOfFiles.splice(index, 1);
      }
    }
  }
  res.send(JSON.stringify(listOfFiles));
});

//Changes the title
app.post('/newTitle', function(req , res){
  var getInfo = JSON.parse(req.body.info);
  lib.setTitle(getInfo[0], getInfo[1]);
  res.redirect('/');
});
//Changes the description
app.post('/newDesc', function(req , res){
  var getInfo = JSON.parse(req.body.info);
  lib.setDescr(getInfo[0], getInfo[1]);
  res.redirect('/');
});
//Adds a new attribute or changes a current one
app.post('/newAttribute', function(req , res){
  var getInfo = JSON.parse(req.body.info);
  var test = lib.addNewAttribute(getInfo[0], getInfo[1], getInfo[2], getInfo[3], getInfo[4]);
  console.log("Added Attribute: "+test);
  addedSuccessfully = test;
  res.redirect('/');
}); 
//COnfirms if attribute change was successful
app.get('/newAttributeConfirmed', function(req, res) {
  res.send(addedSuccessfully);
}); 
//Creates new svg image
app.post('/newSVG', function(req , res){
  var getInfo = JSON.parse(req.body.elements);
  console.log(getInfo);
  var test = lib.newSVGFile(getInfo[0], getInfo[1], getInfo[2]);
  listOfFiles.push(getInfo[0]);
  console.log("File Uploaded: "+test);
  res.sendStatus(200);
}); 
//Creates new circle shape
app.post('/newCircle', function(req , res){
  var getInfo = JSON.parse(req.body.elements);
  console.log(getInfo);
  var test = lib.addNewCirc(getInfo[0], getInfo[1], getInfo[2], getInfo[3], getInfo[4]);
  console.log("Circle Added "+test);
  res.sendStatus(200);
});
//Creates new rectangle shape
app.post('/newRect', function(req , res){
  var getInfo = JSON.parse(req.body.elements);
  console.log(getInfo);
  var test = lib.addNewRect(getInfo[0], getInfo[1], getInfo[2], getInfo[3], getInfo[4], getInfo[5]);
  console.log("Rectangle Added: "+test);
  res.sendStatus(200);
});
//gets file log information
app.get('/svgInfo', function(req , res){
  let svgInfo = new Array();
  for (var i = 0; i < listOfFiles.length; i++) {
    svgInfo.push(lib.getTableInfo(listOfFiles[i]));
  }
  res.send(svgInfo);
});
//reload the page
app.get('/reload', function(req , res){
  res.send(true);
});
//wrapper to get all information about a file (view panel info)
app.get('/imageInfo', function(req , res){
  let imgInfo = new Array();
  console.log("Getting image info...");

  for (var i = 0; i < listOfFiles.length; i++) {
    let img = new Array();
    img.push(lib.getSVGInfo(listOfFiles[i]));
    img.push(lib.getRectInfo(listOfFiles[i]));
    img.push(lib.getCircInfo(listOfFiles[i]));
    img.push(lib.getPathInfo(listOfFiles[i]));
    img.push(lib.getGroupInfo(listOfFiles[i]));
    //console.log(lib.getTitle(listOfFiles[i]));
    img.push(lib.getTitle(listOfFiles[i]));
    //console.log(lib.getDesc(listOfFiles[i]));
    img.push(lib.getDesc(listOfFiles[i]));
    imgInfo.push(img);
  }
  res.send(imgInfo);
}); 
//Get file sizes
app.get('/fileSizes', function(req , res){
  let sizes = new Array();
  for (var i = 0; i < listOfFiles.length; i++) {
    var stats = fs.statSync('uploads/' + listOfFiles[i]);
    var fileSize = Math.round(stats["size"]/1000) + 'KB';
    sizes.push(fileSize);
  }
  res.send(sizes);
});
//Get rectangle attributes
app.get('/rectAttr', function(req , res){
  var string = lib.getRectAttr(rectInfo[0], rectInfo[2]);
  res.send(string);
});
app.post('/sendRect', function(req , res){
  var info = JSON.parse(req.body.info);
  rectInfo = info;
  res.sendStatus(200);
});
//Get circle attributes
app.get('/circAttr', function(req , res){
  var string = lib.getCircAttr(circInfo[0], circInfo[2]);
  res.send(string);
});
app.post('/sendCirc', function(req , res){
  var info = JSON.parse(req.body.info);
  circInfo = info;
  res.sendStatus(200);
});
//Get svg attributes
app.get('/svgAttr', function(req , res){
  var string = lib.getSVGAttr(svgInfo[0]);
  res.send(string);
});
app.post('/sendSVG', function(req , res){
  var info = JSON.parse(req.body.info);
  svgInfo = info;
  res.sendStatus(200);
});
//Get path attributes
app.get('/pathAttr', function(req , res){
  var string = lib.getPathAttr(pathInfo[0], pathInfo[2]);
  res.send(string);
});
app.post('/sendPath', function(req , res){
  var info = JSON.parse(req.body.info);
  pathInfo = info;
  res.sendStatus(200);
});
//Get group attributes
app.get('/groupAttr', function(req , res){
  var string = lib.getGroupAttr(groupInfo[0], groupInfo[2]);
  res.send(string);
});
app.post('/sendGroup', function(req , res){
  var info = JSON.parse(req.body.info);
  groupInfo = info;
  res.sendStatus(200);
});

//Database
let loginInfo;

app.post('/getLoginInfo', function(req, res) {
  login = JSON.parse(req.body.info);
  loginInfo = {
    host : 'dursley.socs.uoguelph.ca', 
    user : login[0], 
    password : login[2], 
    database : login[1]
  }
  res.sendStatus(200);
});

app.get('/checkLogin', function(req, res) {
  if (login.length == 3) {
    res.send({"result": true});
  } else {
    res.send({"result": false});
  }
});

app.get('/checkToEnable', async function(req, res) {
  var numRows = 0;
  try {
    connection = await mysql.createConnection(loginInfo);
    var getFile = `SELECT count(*) as total FROM FILE`;
    const [rows, fieldsF] = await connection.query(getFile); 
    numRows = rows[0].total;
  } catch (e) {
    console.log("Cannot get number of FILE rows: "+e);
  } finally {
    if (connection && connection.end) connection.end();
    if (numRows > 0) {
      res.send({"result":true});
    } else {
      res.send({"result":false});
    }
  }
});

app.get('/connectDB', async function(req, res) {
  console.log("Attempting connetion..");
  try {
    connection = await mysql.createConnection(loginInfo);
    /*connection.query('SET FOREIGN_KEY_CHECKS = 0', function(err, results, fields) {
      if (err) {
        console.log(err.message);
      }
    });
    connection.query('DROP TABLE IF EXISTS FILE', function(err, results, fields) {
      if (err) {
        console.log(err.message);
      }
    });
    connection.query('DROP TABLE IF EXISTS IMG_CHANGE', function(err, results, fields) {
      if (err) {
        console.log(err.message);
      }
    });
    connection.query('DROP TABLE IF EXISTS DOWNLOAD', function(err, results, fields) {
      if (err) {
        console.log(err.message);
      }
    });
    connection.query('SET FOREIGN_KEY_CHECKS = 1', function(err, results, fields) {
      if (err) {
        console.log(err.message);
      }
    });*/
    console.log("Connected sucessfully to " +login[1]);
    let createFILE = `create table if not exists FILE(
                      svg_id int primary key auto_increment,
                      file_name varchar(60)not null,
                      file_title varchar(256) null,
                      file_description varchar(256) null,
                      n_rect int not null,
                      n_circ int not null,
                      n_path int not null,
                      n_group int not null,
                      creation_time timestamp not null,
                      file_size int not null,
                      unique(file_name)
                      )`;
    connection.query(createFILE, function(err, results, fields) {
      if (err) {
        console.log("Could not create FILE: "+err.message);
      }
    });
    let createIMG = `create table if not exists IMG_CHANGE(
                    change_id int primary key auto_increment,
                    change_type varchar(256) not null,
                    change_summary varchar(256) not null,
                    change_time timestamp not null,
                    svg_id int not null,
                    foreign key(svg_id) references FILE(svg_id) on delete cascade
                    )`;
    connection.query(createIMG, function(err, results, fields) {
      if (err) {
        console.log("Could not create IMG_CHANGE: "+err.message);
      }
    });
    let createDOWNLOAD = `create table if not exists DOWNLOAD(
                        download_id int primary key auto_increment,
                        d_descr varchar(256) not null,
                        svg_id int not null,
                        foreign key(svg_id) references FILE(svg_id) on delete cascade
                        )`;
    connection.query(createDOWNLOAD, function(err, results, fields) {
      if (err) {
        console.log("Could not create DOWNLOAD: "+err.message);
      }
    });
    res.status(200).send({"result": "true"});
  } catch(e) {
    console.log("Query error: "+e);
    res.status(200).send({"result": "false"});
  } finally {
    if (connection && connection.end) connection.end();
  }
});

app.post('/addDownload', async function(req, res) {
  var info = JSON.parse(req.body.info);
  console.log("Adding new entry to DOWNLOAD..");
  try {
    connection = await mysql.createConnection(loginInfo);
    var storeDesc = `insert into DOWNLOAD values(null, '`+login[0]+` downloaded `+info+`.', (SELECT svg_id FROM FILE WHERE file_name = '`+info+`'))`;
    connection.query(storeDesc, function(err, results, fields) {
      if (err) {
        console.log(err.message);
      }
    });
  } catch (e) {
    console.log("Could not store file on DOWNLOAD: "+e.message);
  } finally {
    if (connection && connection.end) connection.end();
    res.sendStatus(200);
  }
});

app.post('/addCreate', async function(req, res) {
  var info = JSON.parse(req.body.info);
  console.log("Adding new entry to FILE..");
  try {
    connection = await mysql.createConnection(loginInfo);
    var svg = JSON.parse(lib.getTableInfo(info));
    var t = lib.getTitle(info);
    var d = lib.getDesc(info);
    var stats = fs.statSync('uploads/' + info);
    var fileSize = Math.round(stats["size"]/1000) + 'KB';
    var storeFile = `insert ignore into FILE values(null, '`+info+`', '`+t+`', '`+d+`', `+svg.numRect+`, `+svg.numCirc+ `, `+svg.numPaths+ `, `+svg.numGroups+ `, now(), '`+fileSize+`')`;
    connection.query(storeFile, function(err, results, fields) {
      if (err) {
        console.log(err.message);
      }
    });
  } catch (e) {
    console.log("Could not store file on FILE: "+e.message);
  } finally {
    if (connection && connection.end) connection.end();
    res.sendStatus(200);
  }
});

app.post('/addChange', async function(req, res) {
  var info = JSON.parse(req.body.elements);
  console.log("Adding new entry to IMG_CHANGE..");
  try {
    connection = await mysql.createConnection(loginInfo);
    var storeCHANGE = `insert into IMG_CHANGE values (null, '`+info[1]+`', '`+info[2]+`', now(), (SELECT svg_id FROM FILE WHERE file_name = '`+info[0]+`'))`;
    connection.query(storeCHANGE, function(err, results, fields) {
      if (err) {
        console.log(err.message);
      }
      var stats = fs.statSync('uploads/' + info[0]);
      var fileSize = Math.round(stats["size"]/1000);
      console.log("Updating FILE size..");
      var updateFILE = `update FILE set file_size=`+fileSize+` where file_name='`+info[0]+`'`;
      connection.query(updateFILE, function(err, results, fields) {
        if (err) {
          console.log(err.message);
        }
      });
    });
  } catch (e) {
    console.log("Could not store file on IMG_CHANGE: "+e.message);
  } finally {
    if (connection && connection.end) connection.end();
    res.sendStatus(200);
  }
});

app.get('/store', async function(req, res){
  console.log("Storing files on FILE..");
  var storeFile;
  try {
    connection = await mysql.createConnection(loginInfo);
    for (var i = 0; i < listOfFiles.length; i++) {
      var svg = JSON.parse(lib.getTableInfo(listOfFiles[i]));
      var t = lib.getTitle(listOfFiles[i]);
      var d = lib.getDesc(listOfFiles[i]);
      var stats = fs.statSync('uploads/' + listOfFiles[i]);
      var fileSize = Math.round(stats["size"]/1000) + 'KB';
      storeFile = `insert ignore into FILE values(null, '`+listOfFiles[i]+`', '`+t+`', '`+d+`', `+svg.numRect+`, `+svg.numCirc+ `, `+svg.numPaths+ `, `+svg.numGroups+ `, now(), '`+fileSize+`')`;
      connection.query(storeFile, function(err, results, fields) {
        if (err) {
          console.log(err.message);
        }
      });
    }
  } catch (e) {
    console.log("Could not store all files on FILE: "+e.message);
  } finally {
    console.log("Finished storing files");
    if (connection && connection.end) connection.end();
    res.send({"result":"true"});
  }
});

app.get('/getNumRows', async function(req, res){
  console.log("Sending Alert Data..");
  var getFile = `SELECT count(*) as total FROM FILE`;
  var numFile;
  var getChanges = `SELECT count(*) as total FROM IMG_CHANGE`;
  var numChanges;
  var getDownloads = `SELECT count(*) as total FROM DOWNLOAD`;
  var numDownloads;
  try {
    connection = await mysql.createConnection(loginInfo);
    const [rowsF, fieldsF] = await connection.query(getFile); 
    numFile = rowsF[0].total;
    const [rowsC, fieldsC] = await connection.query(getChanges); 
    numChanges = rowsC[0].total;
    const [rowsD, fieldsD] = await connection.query(getDownloads); 
    numDownloads = rowsD[0].total;
  } catch(e){
    console.log("Could not select counts: "+e.message);
  } finally {
    if (connection && connection.end) connection.end();
    res.send({"file":numFile, "img":numChanges, "d":numDownloads});
  }
});

app.get('/deleteAllRows', async function(req, res){
  var deleteDataFiles = `delete from FILE`;
  var deleteDataChanges = `delete from IMG_CHANGE`;
  var deleteDataDownloads = `delete from DOWNLOAD`;
  try {
    connection = await mysql.createConnection(loginInfo);
    const [rowsF, fieldsF] = await connection.query(deleteDataFiles); 
    const [rowsC, fieldsC] = await connection.query(deleteDataChanges); 
    const [rowsD, fieldsD] = await connection.query(deleteDataDownloads); 
  } catch(e) {
    console.log("Could not clear tables: "+e.message);
  } finally {
    if (connection && connection.end) connection.end();
    res.send({"result":"true"});
  }
});

var q1result;

app.post('/q1FormData', function(req, res) {
  q1result = JSON.parse(req.body.result);
  res.sendStatus(200);
});

app.get('/populateFILETable', async function(req, res) {
  console.log("Getting FILE table data sorted by " +q1result+"..");
  try {
    connection = await mysql.createConnection(loginInfo);
    if (q1result == "name") {
      const [rows, fields] = await connection.execute(`select * from FILE order by file_name`); 
      dataFILE = rows;
    } else if (q1result == "size") {
      const [rows, fields] = await connection.execute(`select * from FILE order by file_size`); 
      dataFILE = rows;
    } else {
      console.log("Error: this is not one of the options to sort FILE data");
    }
  } catch(e) {
    console.log("Could not get Q1 Form Data: "+e.message);
  } finally {
    console.log("Sending FILE data");
    if (connection && connection.end) connection.end();
    res.send(dataFILE);
  }
});

var q2result;

app.post('/q2FormData', function(req, res) {
  q2result = JSON.parse(req.body.result);
  res.sendStatus(200);
});

app.get('/populateQ2Table', async function(req,res) {
  var rowData;
  console.log("Getting FILE table data sorted by " +q2result[0]+" from "+q2result[1]+" to "+q2result[2]+"..");
  try {
    connection = await mysql.createConnection(loginInfo);
    if (q2result[0] == "name") {
      const [rows, fields] = await connection.execute(`select * from FILE where creation_time >= '`+q2result[1]+`' AND creation_time <= '`+q2result[2]+`' order by file_name`); 
      rowData = rows;
    } else if (q2result[0] == "size") {
      const [rows, fields] = await connection.execute(`select * from FILE where creation_time BETWEEN CAST('`+q2result[1]+`' AS DATETIME) AND CAST('`+q2result[2]+`' AS DATETIME) order by file_size`); 
      rowData = rows;
    } else if (q2result[0] == "creation") {
      const [rows, fields] = await connection.execute(`select * from FILE where creation_time >= CAST('`+q2result[1]+`' AS DATETIME) AND creation_time <= CAST('`+q2result[2]+`' AS DATETIME) order by creation_time`); 
      rowData = rows;
    } else {
      console.log("Error: this is not one of the options to sort FILE data");
    }
  } catch(e) {
    console.log("Could not get Q2 Form Data: "+e.message);
  } finally {
    console.log("Sending FILE data");
    if (connection && connection.end) connection.end();
    res.send(rowData);
  }
});

var q3result;

app.post('/q3FormData', function(req, res) {
  q3result = JSON.parse(req.body.result);
  res.sendStatus(200);
});

app.get('/populateQ3Table', async function(req,res) {
  var rowData;
  try {
    connection = await mysql.createConnection(loginInfo);
    if (q3result[0] == "name") {
      var q = `select IMG_CHANGE.*, count(*) as count, FILE.file_name, FILE.file_size from IMG_CHANGE left join FILE on FILE.svg_id = IMG_CHANGE.svg_id where change_time BETWEEN CAST('`+q3result[1]+`' AS DATETIME) AND CAST('`+q3result[2]+`' AS DATETIME) GROUP BY svg_id HAVING COUNT(*) >= 1 order by FILE.file_name`;
      const [rows, fields] = await connection.execute(q); 
      rowData = rows;
    } else if (q3result[0] == "size") {
      var q = `select IMG_CHANGE.*, count(*) as count, FILE.file_name, FILE.file_size from IMG_CHANGE left join FILE on FILE.svg_id = IMG_CHANGE.svg_id where change_time BETWEEN CAST('`+q3result[1]+`' AS DATETIME) AND CAST('`+q3result[2]+`' AS DATETIME) GROUP BY svg_id HAVING COUNT(*) >= 1 order by FILE.file_size`;
      const [rows, fields] = await connection.execute(q); 
      rowData = rows;
    } else if (q3result[0] == "modification") {
      var q = `select IMG_CHANGE.*, count(*) as count, FILE.file_name, FILE.file_size from IMG_CHANGE left join FILE on FILE.svg_id = IMG_CHANGE.svg_id where change_time BETWEEN CAST('`+q3result[1]+`' AS DATETIME) AND CAST('`+q3result[2]+`' AS DATETIME) GROUP BY svg_id HAVING COUNT(*) >= 1 order by change_time desc`;
      const [rows, fields] = await connection.execute(q); 
      rowData = rows;
    }
  } catch(e) {
    console.log("Could not get Q3 Form Data: "+e.message);
  } finally {
    console.log("Sending IMG_CHANGE data");
    if (connection && connection.end) connection.end();
    res.send(rowData);
  }
});

var q4result

app.post('/q4FormData', function(req, res) {
  q4result = JSON.parse(req.body.result);
  res.sendStatus(200);
});

app.get('/populateQ4Table', async function(req,res) {
  console.log("Getting FILE table data sorted by " +q4result[0]+"..");
  try {
    connection = await mysql.createConnection(loginInfo);
    if (q4result[0] == "name") {
      var q = `select * from FILE where `+q4result[1]+` between `+q4result[2]+` and `+q4result[3]+` order by file_name`;
      const [rows, fields] = await connection.execute(q);
      dataFILE = rows;
    } else if (q4result[0] == "size") {
      var q = `select * from FILE where `+q4result[1]+` between `+q4result[2]+` and `+q4result[3]+` order by file_size`;
      const [rows, fields] = await connection.execute(q); 
      dataFILE = rows;
    } else if (q4result[0] == "shapeCount") {
      var q = `select * from FILE where `+q4result[1]+` between `+q4result[2]+` and `+q4result[3]+` order by creation_time`;
      const [rows, fields] = await connection.execute(q); 
      dataFILE = rows;
    } else {
      console.log("Error: this is not one of the options to sort FILE data");
    }
  } catch(e) {
    console.log("Could not get Q4 Form Data: "+e.message);
  } finally {
    console.log("Sending FILE data");
    if (connection && connection.end) connection.end();
    res.send(dataFILE);
  }
});

var q5result;

app.post('/q5FormData', function(req, res) {
  q5result = JSON.parse(req.body.result);
  res.sendStatus(200);
});

app.get('/populateQ5Table', async function(req,res) {
  console.log("Getting DOWNLOAD table data sorted by " +q5result[0]+"..");
  var rowData;
  try {
    connection = await mysql.createConnection(loginInfo);
    if (q5result[0] == "name") {
      var q = `select DOWNLOAD.*, count(*) as count, FILE.file_name, FILE.creation_time from DOWNLOAD left join FILE on FILE.svg_id = DOWNLOAD.svg_id GROUP BY svg_id HAVING COUNT(*) >= 1 order by count desc, FILE.file_name`;
      const [rows, fields] = await connection.execute(q);
      rowData = rows;
    } else if (q5result[0] == "size") {
      var q = `select DOWNLOAD.*, count(*) as count, FILE.file_name, FILE.creation_time from DOWNLOAD left join FILE on FILE.svg_id = DOWNLOAD.svg_id GROUP BY svg_id HAVING COUNT(*) >= 1 order by count desc`;
      const [rows, fields] = await connection.execute(q); 
      rowData = rows;
    } else {
      console.log("Error: this is not one of the options to sort DOWNLOAD data");
    }
  } catch(e) {
    console.log("Could not get Q5 Form Data: "+e.message);
  } finally {
    console.log("Sending DOWNLOAD data");
    if (connection && connection.end) connection.end();
    res.send(rowData);
  }
});

var q6result;

app.post('/q6FormData', function(req, res) {
  q6result = JSON.parse(req.body.result);
  res.sendStatus(200);
});

app.get('/populateQ6Table', async function(req, res) {
  dataFILE = new Array();
  dataFILE.push(q6result[1]);
  console.log("Getting IMG_CHANGE table data sorted by " +q6result[0]+" in file "+q6result[1]+" from "+q6result[2]+" to "+q6result[3]+" with change type of "+q6result[4]+"..");
  try {
    connection = await mysql.createConnection(loginInfo);
    if (q6result[0] == "first") {
      const [rows, fields] = await connection.execute(`select * from IMG_CHANGE where change_time BETWEEN CAST('`+q6result[2]+`' AS DATETIME) AND CAST('`+q6result[3]+`' AS DATETIME) AND svg_id=(SELECT svg_id FROM FILE WHERE file_name = '`+q6result[1]+`') AND change_type='`+q6result[4]+`' order by change_time desc`); 
      dataFILE.push(rows);
    } else if (q6result[0] == "last") {
      const [rows, fields] = await connection.execute(`select * from IMG_CHANGE where change_time BETWEEN CAST('`+q6result[2]+`' AS DATETIME) AND CAST('`+q6result[3]+`' AS DATETIME) AND svg_id=(SELECT svg_id FROM FILE WHERE file_name = '`+q6result[1]+`') AND change_type='`+q6result[4]+`' order by change_time asc`); 
      dataFILE.push(rows);
    } else {
      console.log("Error: this is not one of the options to sort IMG_CHANGE data");
    }
  } catch(e) {
    console.log("Could not get Q6 Form Data: "+e.message);
  } finally {
    console.log("Sending IMG_CHANGE data");
    if (connection && connection.end) connection.end();
    res.send(dataFILE);
  }
});

var changedFileName;

app.post('/pushChangeTypeFile', function(req, res) {
  changedFileName = JSON.parse(req.body.result);
  res.sendStatus(200);
});

app.get('/getChangeTypes', async function(req, res) {
  var changeTypes;
  try {
    connection = await mysql.createConnection(loginInfo);
    const [rows, fields] = await connection.execute(`select change_type from IMG_CHANGE where svg_id=(SELECT svg_id FROM FILE WHERE file_name = '`+changedFileName+`')`); 
    changeTypes = rows;
  } catch(e) {
    console.log("Could not get change_type from IMG_CHANGE: "+e.message);
  } finally {
    console.log("Sending FILE data");
    if (connection && connection.end) connection.end();
    res.send(changeTypes);
  }
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);