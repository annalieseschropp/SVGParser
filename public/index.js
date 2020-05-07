//Annaliese Schropp
//1013472
//CIS 2750 A4

// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    var cur_file;
    var cur_file_add;
    var cur_shape;
    var names;
    var fileNames = new Array();
    var numFiles;
    var table_body = '<table id="fileLog" class="table">';
    var dbTable_body = '';
    var image_body = '';
    var title_body = '';
    var desc_body = '';
    var db_body = '';
    table_body += '<thead class="thead-dark">';
    table_body += '<tr>';
    table_body += '<th scope="col">Image</th>';
    table_body += '<th scope="col">File Name</th scope="col">';
    table_body += '<th scope="col">File Size</th>';
    table_body += '<th scope="col">Rectangles</th>';
    table_body += '<th scope="col">Circles</th>';
    table_body += '<th scope="col">Paths</th>';
    table_body += '<th scope="col">Groups</th>';
    table_body += '</tr>'; 
    table_body += '</thead>';
    table_body += '<tbody id="fileList" scrollbars="yes">';

    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/checkLogin',
        success: function (data) {
            var info = JSON.parse(data.result);
            if (info == true) {
                $("#loginModal").modal('hide');
                checkButtons();
            } else {
                $("#loginModal").modal('show');
            }
        }
    });

    //Submit the login and attempt the 
    $('#loginForm').on('submit',function(e){
        e.preventDefault();
        var usernameText = document.getElementById('usernameInput');
        var username = usernameText.value;
        var dbText = document.getElementById('dbInput');
        var db = dbText.value;
        var passwordText = document.getElementById('passwordInput');
        var password = passwordText.value;
        var loginInfo = new Array();
        loginInfo.push(username);
        loginInfo.push(db);
        loginInfo.push(password);
        var stringOfInfo = JSON.stringify(loginInfo);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {info: stringOfInfo},
            url: '/getLoginInfo',
            success: function (data) {
            }
        });
        loginModal(username);
    });

    function loginModal(username) {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/connectDB',
            success: function (data) {
                var info = JSON.parse(data.result);
                if (info == true) {
                    console.log('Connection created sucessfully');
                    $("#loginModal").modal('hide');
                    welcome_body = '<P align=right>Welcome '+username+'!</P>';
                    $('#welcomeMessage').html(welcome_body);
                    checkButtons();
                } else {
                    alert("Could not create connection with username, database name and password entered, please try again.");
                }
            }
        });
    }

    function checkButtons() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/checkToEnable',
            success: function (data) {
                var info = JSON.parse(data.result);
                if (info == true) { //FILE has at least 1 row, therefore button is usable
                    const buttonVP = document.getElementById('dropdownMenuButton');
                    buttonVP.disabled = false;
                    const buttonAC = document.getElementById('addComponentWhenEnabled');
                    buttonAC.disabled = false;
                    const buttonQ = document.getElementById('query');
                    buttonQ.disabled = false;
                    const buttonClear = document.getElementById('clearData');
                    buttonClear.disabled = false;
                } else { //button is not usable
                    const buttonVP = document.getElementById('dropdownMenuButton');
                    buttonVP.disabled = true;
                    const buttonAC = document.getElementById('addComponentWhenEnabled');
                    buttonAC.disabled = true;
                    const buttonQ = document.getElementById('query');
                    buttonQ.disabled = true;
                    const buttonClear = document.getElementById('clearData');
                    buttonClear.disabled = true;
                }
            }
        });
    }

    // On page-load AJAX Example
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/files',   //The server endpoint we are connecting to
        data: names,
        success: function (data) {
            console.log("Successful table load");
            numFiles = data.length;
            for (var i = 0; i < data.length; i++) {
                fileNames.push(data[i]);
            }
            
            if (numFiles == 0) {
                document.getElementById('NoFiles').style.visibility = "visible";
                buttonStore = document.getElementById('storeFiles');
                buttonStore.disabled = true;
            } else {
                const buttonStore = document.getElementById('storeFiles');
                buttonStore.disabled = false;
            }
            createTable();
            createDropDown();
            createAddComponentDropDown();
            createQueryList();
        },
        fail: function(error) {
            table_body += '</tbody>';
            table_body+='</table>';
            $('#tableDiv').html(table_body);
            document.getElementById('NoFiles').style.visibility = "visible";
        }
    });
    
    //Creates main file log table
    function createTable () {
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/svgInfo',   //The server endpoint we are connecting to
            success: function (data) {
                console.log("Successfully got SVG info");
                addSVGInfo(data);
            },
            fail: function(error) {
                console.log(error); 
            }
        });
    }

    function addDownload(file) {
        var stringFile = JSON.stringify(file);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {info: stringFile},
            url: '/addDownload',
            success: function (data) {
            }
        });
        getDBAlertNoReload();
    }

    //Populates file lof table
    function addSVGInfo(data) {
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/fileSizes',   //The server endpoint we are connecting to
            success: function (d) {
                console.log("Successfully got sizes");
                let elementIds = [];
                for (var i = 0; i < numFiles; i++) {
                    var elementId = fileNames[i].split('.')[0] + "" + i;
                    elementIds.push(elementId);
                    var size = d[i];
                    var temp = JSON.parse(data[i]);
                    
                    table_body += '<tr>';
            
                    table_body += '<td><a id="' + elementId + '" target="_blank" href='+fileNames[i]+'><img src='+fileNames[i]+' style="max-width:200px;"></a></td>';
                    table_body += '<td><a id="' + elementId + '" target="_blank" href='+fileNames[i]+'>'+fileNames[i]+'</a></td>';
                    table_body += '<td>'+size+'</td>';
                    table_body += '<td>'+temp.numRect+'</td>';
                    table_body += '<td>'+temp.numCirc+'</td>';
                    table_body += '<td>'+temp.numPaths+'</td>';
                    table_body += '<td>'+temp.numGroups+'</td>';
            
                    table_body += '</tr>';
                }
                table_body += '</tbody>';
                table_body+='</table>';
                $('#tableDiv').html(table_body);
                elementIds.forEach(function(elementId) {
                    $(document).on('click', '#'+elementId, function(e) {
                        var fileName = $(this).text();
                        addDownload(fileName)
                    });
                });
            },
            fail: function(error) {
            }
        });
    }

    //Populates dropdown for view panel
    function createDropDown () {
        var dropdown_body = '';
        for (var i = 0; i < numFiles; i++) {
            dropdown_body += '<li><button id=\"getInfo\" value=\"'+i+'\" class="dropdown-item">';
            dropdown_body += fileNames[i];
            dropdown_body += '</button></li>'
        }
        $('#chooseFileDropdown').html(dropdown_body);
    }
    
    //On select for view panel drop down, change the title of button to that name
    $(function() {
        $("#chooseFileDropdown").on('click', 'li button', function(){
          $("#dropdownMenuButton:first-child").text($(this).text());
       });
    });

    //populates view panel based on selection
    $(document).on("click", "#getInfo" , function() {
        image_body = '';
        $('#bigImage').html(image_body);
        image_body += '<td><a href='+$(this).text()+'><img src='+$(this).text()+' style="max-width:800px;"></a></td>';
        $('#bigImage').html(image_body);
        cur_file = $(this).text();
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/imageInfo',   //The server endpoint we are connecting to
            success: function (data) {
                console.log("Successfully got chosen file info");
                var index = fileNames.indexOf(cur_file);
                addImageInfo(data, index);
            },
            fail: function(error) {
                console.log(error); 
            }
        });
    });

    //populates add component drop down of all valid files
    function createAddComponentDropDown () {
        var addC_body = '';
        for (var i = 0; i < numFiles; i++) {
            addC_body += '<li><button id=\"addC\" value=\"'+i+'\" class="dropdown-item">';
            addC_body += fileNames[i];
            addC_body += '</button></li>'
        }
        $('#addCFile').html(addC_body);
    }
    
    //Change button name to file name
    $(function() {
        $("#addCFile").on('click', 'li button', function(){
          $("#dropdownButton:first-child").text($(this).text());
       });
    });

    //reveal forms for adding components on selection
    $(document).on("click", "#addC" , function() {
        document.getElementById('circleForm').style.display = 'inline-block';
        document.getElementById('rectForm').style.display = 'inline-block';
        cur_file_add = $(this).text();
    });

    //populate the title and description editors (modals)
    function addImageInfo (data, index) {
        var title_modal = '';
        var desc_modal = '';
        var component_body = '';
        $('#viewPanelContent').html(component_body);
        title_body = '';
        $('#title').html(title_body);
        desc_body = '';
        $('#desc').html(desc_body);
        var info = data[index];
        for (var i = 0; i < info.length; i++) {
            var temp = JSON.parse(info[i]);
            if (temp) {
                if (i == 0) {
                    //svg
                    component_body += '<tr>';
            
                    component_body += '<td id="svgName">SVG</td>';
                    component_body += '<td></td>';
                    component_body += '<td>'+temp[0].numAttr+'  <button type="button" id="svgAttr" class="btn btn-dark" data-toggle="modal" data-target="#attrModal">Show</button></td>';
        
                    component_body += '</tr>';
                }
                if (i == 1) {
                    //rects
                    for (var j = 0; j < temp.length; j++) {
                        component_body += '<tr>';
            
                        component_body += '<td id="rectName">Rectangle '+(j+1)+'</td>';
                        component_body += '<td>Upper left corner: x ='+temp[j].x+temp[j].units+', y ='+temp[j].y+temp[j].units+'<br>Width: '+temp[j].w+temp[j].units+' Height: '+temp[j].h+temp[j].units+'</td>';
                        component_body += '<td>'+temp[j].numAttr+'  <button type="button" id="rectAttr" value="'+j+'" class="btn btn-dark" data-toggle="modal" data-target="#attrModal">Show</button></td>';
            
                        component_body += '</tr>';
                    }
                }
                if (i == 2) {
                    //circs
                    for (var j = 0; j < temp.length; j++) {
                        component_body += '<tr>';
            
                        component_body += '<td id="circName">Circle '+(j+1)+'</td>';
                        component_body += '<td>Center: cx = '+temp[j].cx+temp[j].units+', cy = '+temp[j].cy+temp[j].units+' Radius: '+temp[j].r+temp[j].units+'</td>';
                        component_body += '<td>'+temp[j].numAttr+'  <button type="button" id="circAttr" value="'+j+'" class="btn btn-dark" data-toggle="modal" data-target="#attrModal">Show</button></td>';
            
                        component_body += '</tr>';
                    }
                }
                if (i == 3) {
                    //paths
                    for (var j = 0; j < temp.length; j++) {
                        component_body += '<tr>';
            
                        component_body += '<td id="pathName">Path '+(j+1)+'</td>';
                        component_body += '<td>Path Data = '+temp[j].d+'</td>';
                        component_body += '<td>'+temp[j].numAttr+'  <button type="button" id="pathAttr" value="'+j+'" class="btn btn-dark" data-toggle="modal" data-target="#attrModal">Show</button></td>';
            
                        component_body += '</tr>';
                    }
                }
                if (i == 4) {
                    //groups
                    for (var j = 0; j < temp.length; j++) {
                        component_body += '<tr>';
            
                        component_body += '<td id="groupName">Group '+(j+1)+'</td>';
                        component_body += '<td>'+temp[j].children+' child elements</td>';
                        component_body += '<td>'+temp[j].numAttr+'  <button type="button" id="groupAttr" value="'+j+'" class="btn btn-dark" data-toggle="modal" data-target="#attrModal">Show</button></td>';
            
                        component_body += '</tr>';
                    }
                }
                if ( i == 5) {
                    title_body += temp;
                    title_modal += temp;
                }
                if (i == 6) {
                    desc_body += temp;
                    desc_modal += temp;
                }
            }
        }
        title_body += '<br><button type="button" id="editTitle" class="btn btn-dark" data-toggle="modal" data-target="#titleModal">Edit</button>';
        desc_body += '<br><button type="button" id="editDesc" class="btn btn-dark" data-toggle="modal" data-target="#descModal">Edit</button>';
        $('#title').html(title_body);
        $('#desc').html(desc_body);
        title_modal += '<br><br><form id="titleForm"><label for="entryBox">Change Title</label><textarea class="form-control rounded-0" id="newTitle" rows="3" maxlength="250"></textarea></form>';
        title_modal += '<br><button id="titleSubmit" type="button" style="text-align:left;" class="btn btn-primary" data-dismiss="modal">Save Changes</button>';
        $('#titleModalContent').html(title_modal);
        desc_modal += '<br><br><form id="descForm"><label for="entryBox">Change Description</label><textarea class="form-control rounded-0" id="newDesc" rows="3" maxlength="250"></textarea></form>';
        desc_modal += '<br><button id="descSubmit" type="button" style="text-align:left;" class="btn btn-primary" data-dismiss="modal">Save Changes</button>';
        $('#descModalContent').html(desc_modal);
        $('#viewPanelContent').html(component_body);
    }

    db_body += '<div style="text-align: center; padding-bottom: 2%;">';
    db_body += '<button type="button" id="storeFiles" class="btn btn-dark" style="margin: 2%;">Store Files</button>';
    db_body += '<button type="button" id="clearData" class="btn btn-dark" style="margin: 2%;">Clear Data</button>';
    db_body += '<button type="button" id="status" class="btn btn-dark" style="margin: 2%;">Status</button>';
    db_body += '<br><div class="dropdown" style="display:inline-block;">';
    db_body += '<button id="query"class="btn btn-dark dropdown-toggle" type="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false" style="margin: 2%;">Execute Query </button>';
    db_body += '<div id="queryOption" class="dropdown-menu" aria-labelledby="dropdownButton">';
    db_body += '<li><button id="q1" class="dropdown-item">Display All Data</button></li>';
    db_body += '<li><button id="q2" class="dropdown-item">Display Created Dates</button></li>';          
    db_body += '<li><button id="q3" class="dropdown-item">Display Modified Dates</button></li>';
    db_body += '<li><button id="q4" class="dropdown-item">Display Shape Counts</button></li>';
    db_body += '<li><button id="q5" class="dropdown-item">Display Downloaded</button></li>';
    db_body += '<li><button id="q6" class="dropdown-item">Display Changes</button></li></div></div></div>';
    $('#databaseButtons').html(db_body);
    
    function createQueryList() {
        var fileOptions_body = '<select id="fileOptions" name="fileOptions">';
        for (var i = 0; i < numFiles; i++) {
            fileOptions_body += '<option value="'+fileNames[i]+'">'+fileNames[i]+'</option>';
        }
        fileOptions_body += '</select>';
        $('#fileList').html(fileOptions_body);
    }

    const fileBtn = document.getElementById("real-file");
    const uploadBtn = document.getElementById("upload");

    //Upload button
    uploadBtn.addEventListener("click", function() {
        fileBtn.click();
    });

    //Event listener for upload button, send data to server
    fileBtn.addEventListener("change", function() {
        if (fileBtn.value) {
            var newFile = fileBtn.files.item(0);
            formdata = new FormData();
            formdata.append('uploadFile', newFile);
            $.ajax({
                type: 'post',
                data: formdata,
                url: '/upload',
                processData: false, 
                contentType: false, 
                success: function (data) {
                    isValid(formdata, newFile.name);
                }
            });        
        }
    });

    //Checks if upload is valid
    function isValid (formdata, newFileName) {
        $.ajax({
            type: 'get',
            url: '/valid',
            success: function (data) {
                if (data == true) {
                    addCreate(newFileName);
                    console.log('File Uploaded Successfully');
                } else {
                    alert("Whoops! That file wasn't valid. Cannot upload.");
                    console.log('Invalid File Upload Attempt');
                    removeFile(formdata);
                }
            }
        });
    }

    function addCreate (newFileName) {
        var stringFile = JSON.stringify(newFileName);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {info: stringFile},
            url: '/addCreate',
            success: function (data) {
            }
        });
        getDBAlert();
    }

    //Upload was not valid so remove from uploads folder
    function removeFile (formdata) {
        $.ajax({
            type: 'post',
            data: formdata,
            url: '/moveFile',
            processData: false, 
            contentType: false, 
            success: function (data) {
                console.log("Successfully Moved the File");
            }
        });
    }

    //Submit the title and update the file
    $(document).on('click', '#titleSubmit', function() {
        var newTitleText = document.getElementById('newTitle');
        var title = newTitleText.value;
        var updateTitle = new Array();
        updateTitle.push(cur_file);
        updateTitle.push(title);
        var stringOfInfo = JSON.stringify(updateTitle);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {info: stringOfInfo},
            url: '/newTitle',
            success: function (data) {
                console.log('Pushed New Title Successfully');
            }
        });
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/reload',
            success: function (data) {
                console.log('Pushed New Title Successfully');
            }
        });
        var addChange = new Array();
        addChange.push(cur_file);
        addChange.push("change title");
        addChange.push("change title in file "+cur_file);
        var addChangeString = JSON.stringify(addChange);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {elements: addChangeString},
            url: '/addChange',
            success: function (data) {
                console.log("Successful Form Submit");
            }
        });
        getDBAlert();
    });
    //Submit the description and update the file
    $(document).on('click', '#descSubmit', function() {
        var newDescText = document.getElementById('newDesc');
        var desc = newDescText.value;
        var updateDesc = new Array();
        updateDesc.push(cur_file);
        updateDesc.push(desc);
        var stringOfInfo = JSON.stringify(updateDesc);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {info: stringOfInfo},
            url: '/newDesc',
            success: function (data) {
                console.log('Pushed New Description Successfully');
            }
        });
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/reload',
            success: function (data) {
                console.log('Pushed New Description Successfully');
            }
        });
        var addChange = new Array();
        addChange.push(cur_file);
        addChange.push("change description");
        addChange.push("change description in file "+cur_file);
        var addChangeString = JSON.stringify(addChange);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {elements: addChangeString},
            url: '/addChange',
            success: function (data) {
                console.log("Successful Form Submit");
            }
        });
        getDBAlert();
    });
    //Create a new svg file and upload it to server
    $('#SVGForm').on('submit',function(e){
        e.preventDefault();
        var newFile = $('#filename').val();
        if (fileNames.includes(newFile)) {
            alert("Whoops! A file with that name already exists!");
        } else if (!(newFile.includes(".svg"))) {
            alert("Whoops! The filename doesn't contain the proper extension! Please include .svg!");
        } else {
            var svgTitle = $('#SVGtitle').val();
            var svgDesc = $('#SVGdesc').val();
            var svgElements = new Array();
            svgElements.push(newFile);
            svgElements.push(svgTitle);
            svgElements.push(svgDesc);
            var string = JSON.stringify(svgElements);
            $.ajax({
                type: 'post',
                dataType: 'json',
                data: {elements: string},
                url: '/newSVG',
                success: function (data) {
                    console.log("Successful SVG Form Submit");
                }
            });
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/reload',
                success: function (data) {
                    console.log('Successful SVG Form Submit');
                }
            });
            addCreate(newFile);
        }
    });
    //Add a new rectangle to specific file
    $('#rectForm').on('submit',function(e){
        e.preventDefault();
        var x = $('#x').val();
        var y = $('#y').val();
        var height = $('#height').val();
        var width = $('#width').val();
        var units = $('#rectUnits').val();
        var rectElements = new Array();
        rectElements.push(cur_file_add);
        rectElements.push(x);
        rectElements.push(y);
        rectElements.push(height);
        rectElements.push(width);
        rectElements.push(units);
        var string = JSON.stringify(rectElements);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {elements: string},
            url: '/newRect',
            success: function (data) {
                console.log("Successful Form Submit");
            }
        });
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/reload',
            success: function (data) {
                console.log('Successful Form Submit');
            }
        });
        var addChange = new Array();
        addChange.push(cur_file_add);
        addChange.push("add rectangle");
        addChange.push("add rectangle in file "+cur_file_add+" at "+x+","+y+" with a height of "+height+" and a width of "+width);
        var addChangeString = JSON.stringify(addChange);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {elements: addChangeString},
            url: '/addChange',
            success: function (data) {
                console.log("Successful Form Submit");
            }
        });
        getDBAlert();
    });
    //Add a new circle to specific file
    $('#circleForm').on('submit',function(e){
        e.preventDefault();
        var cx = $('#cx').val();
        var cy = $('#cy').val();
        var r = $('#r').val();
        var units = $('#circUnits').val();
        var circElements = new Array();
        circElements.push(cur_file_add);
        circElements.push(cx);
        circElements.push(cy);
        circElements.push(r);
        circElements.push(units);
        var string = JSON.stringify(circElements);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {elements: string},
            url: '/newCircle',
            success: function (data) {
                console.log("Successful Form Submit");
            }
        });
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/reload',
            success: function (data) {
                console.log('Successful Form Submit');
            }
        });
        var addChange = new Array();
        addChange.push(cur_file_add);
        addChange.push("add circle");
        addChange.push("add circle in file "+cur_file_add+" at "+cx+","+cy+" with a radius of "+r);
        var addChangeString = JSON.stringify(addChange);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {elements: addChangeString},
            url: '/addChange',
            success: function (data) {
                console.log("Successful Form Submit");
            }
        });
        getDBAlert();
    });

    $("#storeFiles").on("click", function() {
        console.log("Storing files on database..");
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/store',
            success: function (data) {
                var input = JSON.parse(data.result);
                if (input == true) {
                    console.log('Successfully stored files');
                    getDBAlert();
                }
            }
        });
    });

    function getDBAlert() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getNumRows',
            success: function (data) {
                var fileTable = JSON.parse(data.file);
                var imgTable = JSON.parse(data.img);
                var dTable = JSON.parse(data.d);
                alert("The Database has " + fileTable + " files, " + imgTable + " changes, and " + dTable + " downloads.");
                window.location.reload(true);
            }
        });
    }
    function getDBAlertNoReload() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getNumRows',
            success: function (data) {
                var fileTable = JSON.parse(data.file);
                var imgTable = JSON.parse(data.img);
                var dTable = JSON.parse(data.d);
                alert("The Database has " + fileTable + " files, " + imgTable + " changes, and " + dTable + " downloads.");
            }
        });
    }

    $("#status").on("click", function () {
        getDBAlertNoReload();
    });

    $("#clearData").on("click", function () {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/deleteAllRows',
            success: function (data) {
                getDBAlert();
            }
        });
    });

    $("#q1").on("click", function () {
        $("#dbTable").empty();
        document.getElementById('q2Form').style.display = 'none';
        document.getElementById('q3Form').style.display = 'none';
        document.getElementById('q4Form').style.display = 'none';
        document.getElementById('q5Form').style.display = 'none';
        document.getElementById('q6Form').style.display = 'none';
        document.getElementById('q1Form').style.display = 'block';
        $('html, body').animate({
            scrollTop: $('#q1Form').offset().top
        }, 1500);
    });

    $('#q1Form').on('submit',function(e){
        e.preventDefault();
        document.getElementById('q1Form').style.display = 'none';
        var string;
        var radioButtons = document.getElementsByName("q1sortType");
        for(var i = 0; i < radioButtons.length; i++){
            if(radioButtons[i].checked == true) {
                string = JSON.stringify(radioButtons[i].value);
            }
        }
        if (string != null) {
            $.ajax({
                type: 'post',
                dataType: 'json',
                data: {result: string},
                url: '/q1FormData',
                success: function (data) {
                    console.log("Successful Form Submit");
                }
            });
            var frm = document.getElementById('q1Form');
            frm.reset();
            populateFILEtable();
        }
    });

    function populateFILEtable() {
        console.log("Populating..");
        dbTable_body = '<table class="table" id="tableFILE""><thead class="thead-dark"><tr>';
        dbTable_body += '<th scope="col">File Name</th><th scope="col">Title</th><th scope="col">Description</th><th scope="col">Rectangles</th><th scope="col">Circles</th><th scope="col">Paths</th><th scope="col">Groups</th><th scope="col">Created</th><th scope="col">Size</th>';
        dbTable_body += '</tr></thead><tbody>';
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/populateFILETable',
            success: function (data) {
                rowsFILE(data);
            }
        });
    }

    function rowsFILE (rows) {
        for (var i = 0; i < rows.length; i++) {
            dbTable_body += '<tr><td>'+rows[i].file_name+'</td>';
            if (rows[i].file_title == '" "') {
                dbTable_body += '<td></td>';
            } else {
                dbTable_body += '<td>'+rows[i].file_title+'</td>';
            }
            if (rows[i].file_description == '" "') {
                dbTable_body += '<td></td>';
            } else {
                dbTable_body += '<td>'+rows[i].file_description+'</td>';
            }
            var d = new Date(rows[i].creation_time);
            dbTable_body += '<td>'+rows[i].n_rect+'</td>';
            dbTable_body += '<td>'+rows[i].n_circ+'</td>';
            dbTable_body += '<td>'+rows[i].n_path+'</td>';
            dbTable_body += '<td>'+rows[i].n_group+'</td>';
            dbTable_body += '<td>'+d+'</td>';
            dbTable_body += '<td>'+rows[i].file_size+'KB</td></tr>';
        }
        dbTable_body += '</tbody></table>';
        $('#dbTable').html(dbTable_body);
        console.log("Populated FILE table");
    } 

    $("#q2").on("click", function () {
        $("#dbTable").empty();
        document.getElementById('q1Form').style.display = 'none';
        document.getElementById('q3Form').style.display = 'none';
        document.getElementById('q4Form').style.display = 'none';
        document.getElementById('q5Form').style.display = 'none';
        document.getElementById('q6Form').style.display = 'none';
        document.getElementById('q2Form').style.display = 'block';
        $('html, body').animate({
            scrollTop: $('#q2Form').offset().top
        }, 1500);
    });

    function isValidDate(d) {
        return d instanceof Date && !isNaN(d);
      }

    $('#q2Form').on('submit',function(e){
        e.preventDefault();
        document.getElementById('q2Form').style.display = 'none';
        var string = new Array();
        var radioButtons = document.getElementsByName("q2sortType");
        for(var i = 0; i < radioButtons.length; i++){
            if(radioButtons[i].checked == true) {
                string.push(radioButtons[i].value);
            }
        }
        var from = document.getElementById('q2From').value;
        var tempDate = new Date(from);
        var check = isValidDate(tempDate);
        if (check == false) {
            //not a valid date
            document.getElementById('q2Form').style.display = 'block';
            alert("Oops, that was not a correct date entered! Please try again.");
        } else {
            string.push(from);
            var to = document.getElementById('q2To').value;
            var tempToDate = new Date(to);
            check = isValidDate(tempToDate);
            if (check == false) {
                //not valid date
                document.getElementById('q2Form').style.display = 'block';
                alert("Oops, that was not a correct date entered! Please try again.");
            } else {
                string.push(to);
                var stringOfInfo = JSON.stringify(string);
                if (string.length == 3) {
                    $.ajax({
                        type: 'post',
                        dataType: 'json',
                        data: {result: stringOfInfo},
                        url: '/q2FormData',
                        success: function (data) {
                            console.log("Successful Form Submit");
                        }
                    });
                    populateQ2table();
                }
            }
        }
    });

    function populateQ2table() {
        console.log("Populating..");
        dbTable_body = '<table class="table" id="tableFILE""><thead class="thead-dark"><tr>';
        dbTable_body += '<th scope="col">File Name</th><th scope="col">Title</th><th scope="col">Description</th><th scope="col">Rectangles</th><th scope="col">Circles</th><th scope="col">Paths</th><th scope="col">Groups</th><th scope="col">Created</th><th scope="col">Size</th>';
        dbTable_body += '</tr></thead><tbody>';
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/populateQ2Table',
            success: function (data) {
                rowsFILE(data);
            }
        });
    }

    $("#q3").on("click", function () {
        $("#dbTable").empty();
        document.getElementById('q1Form').style.display = 'none';
        document.getElementById('q2Form').style.display = 'none';
        document.getElementById('q4Form').style.display = 'none';
        document.getElementById('q5Form').style.display = 'none';
        document.getElementById('q6Form').style.display = 'none';
        document.getElementById('q3Form').style.display = 'block';
        $('html, body').animate({
            scrollTop: $('#q3Form').offset().top
        }, 1500);
    });

    $('#q3Form').on('submit',function(e){
        e.preventDefault();
        document.getElementById('q3Form').style.display = 'none';
        var string = new Array();
        var radioButtons = document.getElementsByName("q3sortType");
        for(var i = 0; i < radioButtons.length; i++){
            if(radioButtons[i].checked == true) {
                string.push(radioButtons[i].value);
            }
        }
        var from = document.getElementById('q3From').value;
        var tempDate = new Date(from);
        var check = isValidDate(tempDate);
        if (check == false) {
            //not a valid date
            document.getElementById('q2Form').style.display = 'block';
            alert("Oops, that was not a correct date entered! Please try again.");
        } else {
            string.push(from);
            var to = document.getElementById('q3To').value;
            var tempToDate = new Date(to);
            check = isValidDate(tempToDate);
            if (check == false) {
                //not a valid date
                document.getElementById('q2Form').style.display = 'block';
                alert("Oops, that was not a correct date entered! Please try again.");
            } else {
                string.push(to);
                var stringOfInfo = JSON.stringify(string);
                if (string.length == 3) {
                    $.ajax({
                        type: 'post',
                        dataType: 'json',
                        data: {result: stringOfInfo},
                        url: '/q3FormData',
                        success: function (data) {
                            console.log("Successful Form Submit");
                        }
                    });
                    populateCHANGEtable();
                }
            }
        }
    });

    function populateCHANGEtable() {
        console.log("Populating..");
        dbTable_body = '<table class="table" id="tableCHANGE"><thead class="thead-dark"><tr>';
        dbTable_body += '<th scope="col">File Name</th><th scope="col">Size</th><th scope="col">Modification Date</th><th scope="col">Changes</th>';
        dbTable_body += '</tr></thead><tbody>';
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/populateQ3Table',
            success: function (data) {
                rowsCHANGE(data);
            }
        });
    }

    function rowsCHANGE (rows) {
        for (var i = 0; i < rows.length; i++) {
            var d = new Date(rows[i].change_time);
            dbTable_body += '<tr><td>'+rows[i].file_name+'</td>';
            dbTable_body += '<td>'+rows[i].file_size+'KB</td>';
            dbTable_body += '<td>'+d+'</td>';
            dbTable_body += '<td>'+rows[i].count+'</td></tr>';
        }
        dbTable_body += '</tbody></table>';
        $('#dbTable').html(dbTable_body);
        console.log("Populated IMG_CHANGE table");
    }
    
    $("#q4").on("click", function () {
        $("#dbTable").empty();
        document.getElementById('q1Form').style.display = 'none';
        document.getElementById('q2Form').style.display = 'none';
        document.getElementById('q3Form').style.display = 'none';
        document.getElementById('q5Form').style.display = 'none';
        document.getElementById('q6Form').style.display = 'none';
        document.getElementById('q4Form').style.display = 'block';
        $('html, body').animate({
            scrollTop: $('#q4Form').offset().top
        }, 1500);
    });

    $('#q4Form').on('submit',function(e){
        e.preventDefault();
        document.getElementById('q4Form').style.display = 'none';
        var string = new Array();
        var radioButtons1 = document.getElementsByName("q4sortType");
        for(var i = 0; i < radioButtons1.length; i++){
            if(radioButtons1[i].checked == true) {
                string.push(radioButtons1[i].value);
            }
        }
        var radioButtons2 = document.getElementsByName("shapeType");
        for(var i = 0; i < radioButtons2.length; i++){
            if(radioButtons2[i].checked == true) {
                string.push(radioButtons2[i].value);
            }
        }
        var from = document.getElementById('q4minNumShape').value;
        string.push(from);
        var to = document.getElementById('q4maxNumShape').value;
        string.push(to);
        var stringOfInfo = JSON.stringify(string);
        if (string.length == 4) {
            $.ajax({
                type: 'post',
                dataType: 'json',
                data: {result: stringOfInfo},
                url: '/q4FormData',
                success: function (data) {
                    console.log("Successful Form Submit");
                }
            });
            populateQ4table();
        }
    });

    function populateQ4table() {
        console.log("Populating..");
        dbTable_body = '<table class="table" id="tableFILE""><thead class="thead-dark"><tr>';
        dbTable_body += '<th scope="col">File Name</th><th scope="col">Title</th><th scope="col">Description</th><th scope="col">Rectangles</th><th scope="col">Circles</th><th scope="col">Paths</th><th scope="col">Groups</th><th scope="col">Created</th><th scope="col">Size</th>';
        dbTable_body += '</tr></thead><tbody>';
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/populateQ4Table',
            success: function (data) {
                rowsFILE(data);
            }
        });
    }

    $("#q5").on("click", function () {
        $("#dbTable").empty();
        document.getElementById('q1Form').style.display = 'none';
        document.getElementById('q2Form').style.display = 'none';
        document.getElementById('q3Form').style.display = 'none';
        document.getElementById('q4Form').style.display = 'none';
        document.getElementById('q6Form').style.display = 'none';
        document.getElementById('q5Form').style.display = 'block';
        $('html, body').animate({
            scrollTop: $('#q5Form').offset().top
        }, 1500);
    });

    $('#q5Form').on('submit',function(e){
        e.preventDefault();
        document.getElementById('q5Form').style.display = 'none';
        var string = new Array();
        var radioButtons1 = document.getElementsByName("q5sortType");
        for(var i = 0; i < radioButtons1.length; i++){
            if(radioButtons1[i].checked == true) {
                string.push(radioButtons1[i].value);
            }
        }
        var to = document.getElementById('q5displayNum').value;
        string.push(to);
        var stringOfInfo = JSON.stringify(string);
        if (string.length == 2) {
            $.ajax({
                type: 'post',
                dataType: 'json',
                data: {result: stringOfInfo},
                url: '/q5FormData',
                success: function (data) {
                    console.log("Successful Form Submit");
                }
            });
            populateDOWNLOADtable(to);
        }
    });

    function populateDOWNLOADtable(displayNum) {
        console.log("Populating..");
        dbTable_body = '<table class="table" id="tableDOWNLOAD"><thead class="thead-dark"><tr>';
        dbTable_body += '<th scope="col">File Name</th><th scope="col">Summary</th><th scope="col">Creation Date</th><th scope="col">Downloads</th>';
        dbTable_body += '</tr></thead><tbody>';
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/populateQ5Table',
            success: function (data) {
                rowsDOWNLOAD(data, displayNum);
            }
        });
    }

    function rowsDOWNLOAD (rows, displayNum) {
        var loopNum
        if (rows.length > displayNum) {
            loopNum = displayNum;
        } else {
            loopNum = rows.length;
        }
        for (var i = 0; i < loopNum; i++) {
            var d = new Date(rows[i].creation_time);
            dbTable_body += '<tr><td>'+rows[i].file_name+'</td>';
            dbTable_body += '<td>'+rows[i].d_descr+'KB</td>';
            dbTable_body += '<td>'+d+'</td>';
            dbTable_body += '<td>'+rows[i].count+'</td></tr>';
        }
        dbTable_body += '</tbody></table>';
        $('#dbTable').html(dbTable_body);
        console.log("Populated DOWNLOAD table");
    }

    $("#q6").on("click", function () {
        $("#dbTable").empty();
        document.getElementById('q1Form').style.display = 'none';
        document.getElementById('q2Form').style.display = 'none';
        document.getElementById('q3Form').style.display = 'none';
        document.getElementById('q4Form').style.display = 'none';
        document.getElementById('q5Form').style.display = 'none';
        document.getElementById('q6Form').style.display = 'block';
        $('html, body').animate({
            scrollTop: $('#q6Form').offset().top
        }, 1500);
    });

    var fileList;

    $('#q6Form').on('submit',function(e){
        e.preventDefault();
        document.getElementById('q6Form').style.display = 'none';
        fileList = document.getElementById('fileOptions').value;
        if (fileList != null) {
            showSecondaryQ6Form();
        }
    });

    function showSecondaryQ6Form() {
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {result: JSON.stringify(fileList)},
            url: '/pushChangeTypeFile',
            success: function (data) {
            }
        });
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/getChangeTypes',
            success: function (data) {
                showForm(data);
            }
        });
    }

    function showForm(data) {
        if (data.length == 0) {
            alert("There have been no changes to that file! Please choose a different file.");
        } else {
            var q6SecondForm = '<select id="typeOptions" name="typeOptions">';
            for (var i = 0; i < data.length; i++) {
                q6SecondForm += '<option value="'+data[i].change_type+'">'+data[i].change_type+'</option>';
            }
            q6SecondForm += '</select>';
            $('#changeList').html(q6SecondForm);
            document.getElementById('q6FormInfo').style.display = 'block';
        }
    }

    $('#q6FormInfo').on('submit',function(e) {
        e.preventDefault();
        document.getElementById('q6FormInfo').style.display = 'none';
        var string = new Array();
        var radioButtons = document.getElementsByName("q6sortType");
        for(var i = 0; i < radioButtons.length; i++){
            if(radioButtons[i].checked == true) {
                string.push(radioButtons[i].value);
            }
        }
        var fileList = document.getElementById('fileOptions').value;
        string.push(fileList);
        var from = document.getElementById('q6From').value;
        var tempDate = new Date(from);
        var check = isValidDate(tempDate);
        if (check == false) {
            //not a valid date
            document.getElementById('q6FormInfo').style.display = 'block';
            alert("Oops, that was not a correct date entered! Please try again.");
        } else {
            string.push(from);
            var to = document.getElementById('q6To').value;
            var tempToDate = new Date(to);
            check = isValidDate(tempToDate);
            if (check == false) {
                //not a valid date
                document.getElementById('q6FormInfo').style.display = 'block';
                alert("Oops, that was not a correct date entered! Please try again.");
            } else {
                string.push(to);
                var typeList = document.getElementById('typeOptions').value;
                string.push(typeList);
                var stringOfInfo = JSON.stringify(string);
                if (string.length == 5) {
                    $.ajax({
                        type: 'post',
                        dataType: 'json',
                        data: {result: stringOfInfo},
                        url: '/q6FormData',
                        success: function (data) {
                            console.log("Successful Form Submit");
                        }
                    });
                    populateCtable();
                }
            }
        }
    });

    function populateCtable() {
        console.log("Populating..");
        dbTable_body = '<table class="table" id="tableCHANGE"><thead class="thead-dark"><tr>';
        dbTable_body += '<th scope="col">File Name</th><th scope="col">Change Type</th><th scope="col">Change Description</th><th scope="col">Change Date</th>';
        dbTable_body += '</tr></thead><tbody>';
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/populateQ6Table',
            success: function (data) {
                rowsC(data);
            }
        });
    }

    function rowsC (rows) {
        var info = rows[1];
        for (var i = 0; i < info.length; i++) {
            var d = new Date(info[i].change_time);
            dbTable_body += '<tr><td>'+rows[0]+'</td>';
            dbTable_body += '<td>'+info[i].change_type+'</td>';
            dbTable_body += '<td>'+info[i].change_summary+'</td>';
            dbTable_body += '<td>'+d+'</td></tr>';
        }
        dbTable_body += '</tbody></table>';
        $('#dbTable').html(dbTable_body);
        console.log("Populated IMG_CHANGE table");
    }

    //Make svg form visible
    $("#createSVG").on("click", function() {
        document.getElementById('SVGForm').style.visibility = "visible";
    });
    //make adding circle form visible
    $("#addCircle").on("click", function() {
        document.getElementById('rectForm').style.display = 'none';
        document.getElementById('circleForm').style.display = 'block';
    });
    //make adding rectangle visible
    $("#addRect").on("click", function() {
        document.getElementById('circleForm').style.display = 'none';
        document.getElementById('rectForm').style.display = 'block';
    });
    //shows the svg attributes list, allows you to edit, add and scale this shape
    $(document).on('click', '#svgAttr', function() {
        var $row = $(this).closest("tr")
        var $column = $row.find("#svgName")
        var cName = $column.text();
        cur_shape = cName;
        var stringArray = new Array();
        stringArray.push(cur_file);
        stringArray.push(cur_shape);
        var jsonString = JSON.stringify(stringArray);
        $.ajax({
            type: 'post',
            dataType: 'json',      
            url: '/sendSVG',
            data: {info: jsonString},
            success: function (data) {
                console.log("Successfully send SVG Data");
            },
            fail: function(error) {
            }
        });
        $.ajax({
            type: 'get',
            dataType: 'json',      
            url: '/svgAttr',
            success: function (data) {
                console.log("Successfully got SVG Attributes");
                showAttributes(data);
            },
            fail: function(error) {
            }
        });
    });
    //shows the circle attributes list, allows you to edit, add and scale this shape
    $(document).on('click', '#circAttr', function() {
        var $row = $(this).closest("tr")
        var $column = $row.find("#circName")
        var cName = $column.text();
        cur_shape = cName;
        string = cName.split(" ");
        var stringArray = new Array();
        stringArray.push(cur_file);
        for(var i = 0; i < string.length; i++){
            stringArray.push(string[i]);
        }
        var jsonString = JSON.stringify(stringArray);
        $.ajax({
            type: 'post',
            dataType: 'json',      
            url: '/sendCirc',
            data: {info: jsonString},
            success: function (data) {
                console.log("Successfully send Circle Data");
            },
            fail: function(error) {
            }
        });
        $.ajax({
            type: 'get',
            dataType: 'json',      
            url: '/circAttr',
            success: function (data) {
                console.log("Successfully got Circle Attributes");
                showAttributes(data);
            },
            fail: function(error) {
            }
        });
    });
    //shows the path attributes list, allows you to edit, add and scale this shape
    $(document).on('click', '#pathAttr', function() {
        var $row = $(this).closest("tr")
        var $column = $row.find("#pathName")
        var pName = $column.text();
        cur_shape = pName;
        string = pName.split(" ");
        var stringArray = new Array();
        stringArray.push(cur_file);
        for(var i = 0; i < string.length; i++){
            stringArray.push(string[i]);
        }
        console.log(stringArray);
        var jsonString = JSON.stringify(stringArray);
        $.ajax({
            type: 'post',
            dataType: 'json',      
            url: '/sendPath',
            data: {info: jsonString},
            success: function (data) {
                console.log("Successfully send Path Data");
            },
            fail: function(error) {
            }
        });
        $.ajax({
            type: 'get',
            dataType: 'json',      
            url: '/pathAttr',
            success: function (data) {
                console.log("Successfully got Path Attributes");
                showAttributes(data);
            },
            fail: function(error) {
            }
        });
    });
    //shows the group attributes list, allows you to edit, add and scale this shape
    $(document).on('click', '#groupAttr', function() {
        var $row = $(this).closest("tr")
        var $column = $row.find("#groupName")
        var gName = $column.text();
        cur_shape = gName;
        string = gName.split(" ");
        var stringArray = new Array();
        stringArray.push(cur_file);
        for(var i =0; i < string.length; i++){
            stringArray.push(string[i]);
        }
        var jsonString = JSON.stringify(stringArray);
        $.ajax({
            type: 'post',
            dataType: 'json',      
            url: '/sendGroup',
            data: {info: jsonString},
            success: function (data) {
                console.log("Successfully send Group Data");
            },
            fail: function(error) {
            }
        });
        $.ajax({
            type: 'get',
            dataType: 'json',      
            url: '/groupAttr',
            success: function (data) {
                console.log("Successfully got Group Attributes");
                showAttributes(data);
            },
            fail: function(error) {
            }
        });
    });
    //shows the rectangle attributes list, allows you to edit, add and scale this shape
    $(document).on('click', '#rectAttr', function() {
        var $row = $(this).closest("tr")
        var $column = $row.find("#rectName")
        var rName = $column.text();
        cur_shape = rName;
        string = rName.split(" ");
        var stringArray = new Array();
        stringArray.push(cur_file);
        for(var i = 0; i < string.length; i++){
            stringArray.push(string[i]);
        }
        var jsonString = JSON.stringify(stringArray);
        $.ajax({
            type: 'post',
            dataType: 'json',      
            url: '/sendRect',
            data: {info: jsonString},
            success: function (data) {
                console.log("Successfully send Rect Data");
            },
            fail: function(error) {
            }
        });
        $.ajax({
            type: 'get',
            dataType: 'json',      
            url: '/rectAttr',
            success: function (data) {
                console.log("Successfully got Rect Attributes");
                showAttributes(data);
            },
            fail: function(error) {
            }
        });
    });
    //populates the attributes modal to display to user
    function showAttributes (data) {
        var attributes_body = '';
        $('#attrModalContent').html(attributes_body);
        attributes_body += '<table class="table" id="attrList"><thead class="thead-dark"><tr><th scope="col">Attribute</th><th scope="col">Value</th><th scope="col"></thead><tbody id="attrC" scrollbars="yes">';
        for (var i = 0; i < data.length; i++) {
            attributes_body += '<tr>';
            attributes_body += '<td id="nameAttr">'+data[i].name+'</td>';
            attributes_body += '<td id="valueAttr">'+data[i].value+'</td>'; 
            attributes_body += '</tr>';
        }
        attributes_body += '</tbody></table>';
        attributes_body += '<br><b>Add/Change Attribute</b><form id="attrForm"><div class="row"><div class="col"><input type="text" class="form-control" placeholder="Name" id="newName"></div><div class="col"><input type="text" class="form-control" placeholder="Value" id="newValue"></div></div>';
        attributes_body += '<br><button id="attrSubmit" type="button" style="text-align:left;" class="btn btn-primary" data-dismiss="modal">Add/Change</button></form>';
        var properties = cur_shape.split(" ");
        if (properties != "SVG") {
            attributes_body += '<br><button id="getScaleForm" type="button" class="btn btn-primary" data-dismiss="modal">Scale</button>';
        }

        $('#attrModalContent').html(attributes_body);          
    }

    //shows the scaling modal to scale a shape
    $(document).on('click', '#getScaleForm', function() {
        var scale_body = '';
        $('#scaleModalContent').html(scale_body);
        scale_body += '<b>Scale Shape By</b><form id="scaleShapeForm"><input id="scaleNum" name="num" type="number" class="form-control" placeholder="Ex. 2"/>';
        scale_body += '<br><button id="scaleSubmit" type="button" style="text-align:left;" class="btn btn-primary">Scale</button></form>';
        $('#scaleModalContent').html(scale_body);
        $('#scaleModal').modal('show');

    });
    //Submits new attribute to file
    $(document).on('click', '#attrSubmit', function() {
        var newNameText = document.getElementById('newName');
        var newValueText = document.getElementById('newValue');
        var name = newNameText.value;
        var value = newValueText.value;
        var properties = cur_shape.split(" ");
        var updateAttr = new Array();
        updateAttr.push(cur_file);
        for(var i = 0; i < properties.length; i++){
            updateAttr.push(properties[i]);
        }
        updateAttr.push(name);
        updateAttr.push(value);
        var stringOfInfo = JSON.stringify(updateAttr);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {info: stringOfInfo},
            url: '/newAttribute',
            success: function (data) {
                console.log('Pushed New Attribute Successfully');
            }
        });
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/newAttributeConfirmed',
            success: function (data) {
                if (data == true) {
                    console.log('Pushed New Attribute Successfully');
                    addAttrChange(cur_file, name, value, properties);
                } else {
                    alert("Whoops! That attribute wasn't valid!");
                }
            }
        });
    });

    function addAttrChange(cur_file, name, value, properties) {
        var addChange = new Array();
        addChange.push(cur_file);
        addChange.push("add/change attribute");
        addChange.push("add/change attribute in file "+cur_file+". "+name+" = "+value+" in "+properties[0]+" "+properties[1]);
        var addChangeString = JSON.stringify(addChange);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {elements: addChangeString},
            url: '/addChange',
            success: function (data) {
                console.log("Successful Form Submit");
            }
        });
        getDBAlert();
    }

    //Submits the scaling attribute to the shape
    $(document).on('click', '#scaleSubmit', function() {
        var scale = document.getElementById('scaleNum');
        var scaleNum = parseFloat(scale.value);
        console.log(jQuery.type(scaleNum));
        if (jQuery.type(scaleNum) == 'number') {
            var name = 'transform';
            var value = 'scale('+scaleNum+")";
            var properties = cur_shape.split(" ");
            var updateAttr = new Array();
            updateAttr.push(cur_file);
            for(var i = 0; i < properties.length; i++){
                updateAttr.push(properties[i]);
            }
            updateAttr.push(name);
            updateAttr.push(value);
            var stringOfInfo = JSON.stringify(updateAttr);
            console.log(stringOfInfo);
            $.ajax({
                type: 'post',
                dataType: 'json',
                data: {info: stringOfInfo},
                url: '/newAttribute',
                success: function (data) {
                    console.log('Pushed New Attribute Successfully');
                }
            });
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/newAttributeConfirmed',
                success: function (data) {
                    if (data == true) {
                        console.log('Scaled Successfully');
                        addScaleChange(cur_file, scaleNum, properties);
                    } else {
                        alert("Whoops! That wasn't valid!");
                    }
                }
            });
        } else {
            alert("Whoops! That wasn't valid! Please enter a number.");
        }
    });

    function addScaleChange(cur_file, scaleNum, properties) {
        var addChange = new Array();
        addChange.push(cur_file);
        addChange.push("change scale");
        addChange.push("change scale in file "+cur_file+". Scaled by "+scaleNum+" in "+properties[0]+" "+properties[1]);
        var addChangeString = JSON.stringify(addChange);
        $.ajax({
            type: 'post',
            dataType: 'json',
            data: {elements: addChangeString},
            url: '/addChange',
            success: function (data) {
                console.log("Successful Form Submit");
            }
        });
        getDBAlert();
    }
});