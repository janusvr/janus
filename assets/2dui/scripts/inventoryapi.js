/*
  Beware all ye who enter here. Seriously. This code is a beast. I plan on refactoring
  it in the near future, adding promises and crap to fix the massive amounts of
  callback hell. Additionally, it'll make adding tracking current progres of tasks
  a total breeze.

  Also the redundancy...there's a lot of it. Deal with it.
*/

/*
function ProgressManager() {
  Dispatcher.call(this);
  var tasks = [];
  this.addTask = function(fn, args, cb, label) {
    label = label || 'Performing unknown task';
  }
}


function Task() {
  Dispatcher.call(this);

  this.task = function() {}
  this.taskArgs = [];
  this.cb = function() {};

  this.execute = function() {
    this.dispatch('start');
    this.task.apply(null, args);

    ....there's no way to know if this shit's finished. I need promises, fuck this.
  }

  this.setTask = function(task, args) {
    this.task = task;
    this.taskArgs = args;
    this.cb = cb;
  }
}
*/
function log(a) {
  console.log.apply(console, arguments);
  parent.logToConsole(a);
//  alert(a)
}

window.InventoryAPI = new (function InventoryAPI() {
  var MimeMap = {
    'image/jpeg' : 'jpg',
    'image/jpg' : 'jpg',
    'image/gif' : 'gif',
    'image/png' : 'png'
  };

  Dispatcher.call(this);

  var apiProxies = parseProxies();//['http://sduck.sytes.net:80/inventory'];
  function parseProxies()
  {
    var proxies = parent.window.janus.getsetting('inventoryservers');
    if (proxies == "" || proxies == null)
    {
      proxies = "http://sduck.sytes.net:80/inventory";
      parent.window.janus.setsetting('inventoryservers', proxies);
    }
    proxies = proxies.replace(" ","").split(",");
    var new_proxies = [];
    for (var i = 0; i < proxies.length; i++)
    {
      proxy = proxies[i];
      if (proxy != "")
      {
        new_proxies.push(proxy);
      }
    }
    return new_proxies;
  }
  
  function getAPIUrl() {
    apiProxies = parseProxies();
    return apiProxies[Math.floor(Math.random() * apiProxies.length)];
  }

  var pendingUploads = [];
  window.addEventListener('message', function(ev) {
    if (!pendingUploads[ev.data.id])
      return;

    log('Data! ' + JSON.stringify(ev.data));

    pendingUploads[ev.data.id](ev.data.data);
    delete pendingUploads[ev.data.id];

    //document.getElementById(f + ev.data.id).remove();
  })

  var nextFileId = 0;
  function uploadFile(file, cb) {
    var self = this;
    var formData = new FormData();
    formData.append('file', file);
    var fileId = nextFileId++;

    $.ajax({
      url : getAPIUrl() + '/add',
      type : 'POST',
      data : formData,
      cache : false,
      contentType : false,
      processData : false,
      dataType : 'json',
      xhr : function() {
        var xhr = new XMLHttpRequest();
        //Upload progress
        xhr.upload.addEventListener("progress", function(ev) {
          var data = {
            loaded : ev.loaded
          }

          if (ev.lengthComputable) {
            data.total = ev.total;
            data.percentComplete = ev.loaded / ev.total;
          }

          self.dispatch('uploadprogress', data);
        }, false);

        return xhr;
      },
      beforeSend: function() {
        self.dispatch('uploadstart', {
          id : fileId
        })
        console.log('upload start');
      },

      success : function(data) {
        console.log('upload success!');
        cb(data);
        self.dispatch('uploadcomplete', {
          id : fileId,
          hash : data.hash
        });
      },

      error : function(error) {
        log('Error :( ' + JSON.stringify(error));
		parent.shownotification('Error uploading file to Inventory!','notifications/error.png','null','#442332')
        console.log('upload error', error);
        self.dispatch('uploaderror', {
          id : fileId,
          error : error
        })
      }
    });
  }

  uploadFile = uploadFile.bind(this);

  function getJSON(hash, cb) {
    getFile(hash, cb, '&json=1')
  }

  function getFile(hash, cb, params) {
    params = params || '';
    var id = Math.random();
    pendingUploads[id] = cb;

    /*var frame = document.createElement('iframe');
    frame.id = 'f' + id;
    frame.src = 'http://strandedin.space:8082/' + hash + '?frame=' + id + params;
    document.body.appendChild(frame);*/

  }

  var inventoryJSON;
  var inventoryHash;
  var fileCache = {};
  var fileParentCache = {};

  // Load inventory from IPFS, if it doesn't exist, create it.
  function loadInventory(cb) {

    log('loading...');
    var inventoryHash = getCookie('inventoryHash');
    log('Hash ' + inventoryHash);
    if (!inventoryHash || inventoryHash == 'undefined' || inventoryHash == 'null') {
      log('No inventory exists');
      inventoryJSON = generateInventory();

      saveInventory(function() {
        // Try loading again :D
        loadInventory(cb);
      });
      return;
    }

    $.getJSON(getAPIUrl() + '/catjson/' +inventoryHash, function(data) {

      log('Inventory loaded!', data);
      inventoryJSON = data;

      cacheInventory(inventoryJSON);
      cb();
    }.bind(this))
  }

  // Save our current inventory to IPFS and store the hash in a cookie
  var syncCount = 0;
  function saveInventory(cb) {
    cb = cb || function(){};

    console.log('saving', inventoryJSON);
    if (syncCount == 0)
      window.InventoryAPI.dispatch('syncstart');

    syncCount++;

    $.post(getAPIUrl() + '/addjson', { json : JSON.stringify(inventoryJSON) }, function(data) {
      syncCount--;
      if (syncCount == 0)
        window.InventoryAPI.dispatch('syncend');

      if (data.error) {
        cb(data.error);
        return
      }

      log('Inventory saved to', data.hash);

      inventoryHash = data.hash;
      setCookie('inventoryHash', data.hash, 10000);
      cb();
    })
  }

  function generateLocalHash() {
    var chars = '1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    var len = chars.length;
    var str = '';

    while (str.length < 46) {
      str += chars.substr(Math.floor(Math.random() * len)-1, 1);
    }

    return str;
  }

  function generateDirHash() {
    return 'dir_' + generateLocalHash();
  }

  function generateInventory() {
    return {
      hash : generateDirHash(),
      children : []
    };
  }


  function cacheInventory(node) {
    cacheFile(node);

    if (!node.children)
      return;

    for (var i = 0, il = node.children.length; i < il; ++i) {
      cacheInventory(node.children[i]);
      fileParentCache[node.children[i].hash] = node;
    }
  }

  function cacheFile(node, parent) {
    fileCache[node.hash] = node;
    if (parent)
      fileParentCache[node.hash] = parent;
  }

  function getAbsoluteURL(src) {
    if (src.search(/^[a-z]*:\/\//) == -1) {
      // Relative path
      var url = window.janus.currenturl();

      if (url.split('/').length == 3) {
        url += '/';
      }

      if (src.substr(0,1) == '/') {
        // URL is relative to domain base
        url = url.split('/').slice(0,3).join('/');
      } else {
        // URL is relative to folder
        var toks = url.split('/');
        var top = toks.pop();

        // *Very* naive method of determining folder vs file URL.
        if (top.split('.').length > 1) {
          url = toks.join('/');
        }

        // Make sure it ends in / ^^
        if (url.substr(url.length-1,1) != '/')
          url += '/';
      }
      url += src;

    } else {
      url = src;
    }

    return url;
  }

  this.getInventory = function() {
    return inventoryJSON;
  }

  // Returns the node of the new asset after upload
  this.addFromAssetObject = function(asset, parentHash, cb) {
    var toks = asset.src.split('.')

    var fileName;
    var fileType;

    if (toks.length == 1) {
      fileName = toks[0];
    } else {
      fileType = toks.pop();

      if (fileType == 'gz')
        fileType = toks.pop() + '.gz';

      fileName = toks.join('.');
    }

    switch (fileType) {
      case 'obj':
        uploadRemoteOBJ(asset, function(objHash, mtlHash) {
          //alert('Hashes ' + objHash + ' ' + mtlHash);
          var meta = {};

          if (mtlHash)
            meta.mtl = mtlHash;

          createNode(fileType, objHash, parentHash, meta, function(err, node) {
            cb(err, node);
          });
        });
      break;

      case 'dae':
        uploadRemoteDAE(asset, function(daeHash) {
          createNode(fileType, daeHash, parentHash, cb);
        });
      break;

      default:
        uploadRemoteAsset(asset, function(hash) {
          createNode(fileType, hash, parentHash, cb);
        });
      break;
    }
  }

  function uploadRemoteAsset(asset, cb) {
    var url = getAbsoluteURL(asset.src);

    getRemoteBlob(url, function(err, blob) {
      uploadFile(blob, function(data) {
        cb(data.hash);
      });
    })
  }

  function resolveURL(basePath, src) {
    if (src.search(/^[a-z]*:\/\//) == -1) {
      if (src.substr(0,1) == '/') {
        src = window.janus.currenturl() + src;
      } else {
        src = basePath + src;
      }
    }
    return src;
  }

  function uploadImages(images, cb) {
    var mapHashCache = {};
    var hashList = {};

    var currentImage;
    function uploadImage(src, cb) {
      currentImage = src;
      if (mapHashCache[src]) {
        cb({data: mapHashCache[src]});
        return;
      }

      getRemoteBlob(src, function(err, blob) {
        uploadFile(blob, uploadCallback);
      });
    }

    function uploadCallback(data) {

      hashList[currentImage] = data.hash;
      mapHashCache[currentImage] = data.hash;

      var next = images.pop();

      if (next)
        uploadImage(next, uploadCallback);
      else cb(hashList);
    }

    uploadImage(images.pop(), uploadCallback);
  }

  function uploadRemoteDAE(asset, cb) {

    var basePath = getAbsoluteURL(asset.src);
    basePath = basePath.split('/');
    basePath.pop();
    basePath = basePath.join('/') + '/';

    var assetSrc = getAbsoluteURL(asset.src);
    getRemote(assetSrc, function(xml) {
      if (typeof xml == 'string')
        xml = $.parseXML(xml);
      var $xml = $(xml);
      var imageNodes = $xml.find('image init_from');

      var srcToNode = {};
      var urls = [];

      imageNodes.each(function(index, element) {
        var src = $(element).text();
        src = resolveURL(basePath, src);
        urls.push(src);
        srcToNode[src] = $(element);
      })

      uploadImages(urls, function(hashMap) {
        for (var src in hashMap) {
          var hash = hashMap[src];
          srcToNode[src].text('/ipfs/' + hash);
        }

        var colladaData = $('<div/>').append($xml.children()[0]).html()

        var blob = new Blob([colladaData], { type: 'model/vnd.collada+xml' });
        uploadFile(blob, function(data) {
          cb(data.hash);
        })
      })
    })
  }

  function uploadRemoteOBJ(asset, cb) {
    var url = getAbsoluteURL(asset.src);


    uploadRemoteAsset(asset, function(objHash) {
      if (asset.mtl) {
        var url = getAbsoluteURL(asset.mtl);

        var fr = new FileReader();
        fr.addEventListener("loadend", function() {
          var data = fr.result;
          uploadMtl(data, asset, function(newData) {

            var blob = new Blob([newData], {type: 'text/plain'});

            uploadFile(blob, function(data) {
              var mtlHash = data.hash;
              cb(objHash, mtlHash);
            })
          });
        }, false);


        getRemoteBlob(url, function(err, blob) {
          fr.readAsText(blob);
        });
      } else if (asset.tex0) {
        //alert('Texture attribute based OBJs not yet supported, please use MTL');
        cb(objHash);
      }
    });
  }

  function uploadMtl(text, asset, maps, cb) {
    if (!cb) {
      cb = maps;
      maps = null;
    }

    var basePath = getAbsoluteURL(asset.src);
    basePath = basePath.split('/');
    basePath.pop();
    basePath = basePath.join('/') + '/';


    var lines = text.split("\n");
    var args = {
      '-blendu' : 1,
      '-blendv' : 1,
      '-cc' : 1,
      '-clamp' : 1,
      '-mm' : 2,
      '-o' : 3,
      '-s' : 3,
      '-t' : 3,
      '-textres' : 1,
      '-imfchan' : 1,
      '-type' : 1,
    }

    var mapCmds = ['map_ka', 'map_kd', 'map_ks', 'map_ns', 'map_d', 'decal',
      'disp', 'bump', 'map_bump', 'refl'];

    var matLines = [];
    var line;
    var i,j,k,il,jl,kl;

    for (i = 0, il = lines.length; i < il; i++) {
      line = lines[i].trim().toLowerCase();

      for (j = 0, jl = mapCmds.length; j < jl; ++j) {
        if (line.substr(0, mapCmds[j].length) == mapCmds[j]) {
          matLines.push(i);
          break;
        }
      }
    }

    var mapHashCache = [];
    function uploadMaps(cb) {
      //alert('lines left: ' + matLines);
      var index = matLines.pop();

      if (!index) {
        cb();
        return; // All done!
      }

      var line = lines[index];
      var oldLine = line;

      var toks = line.split(' ');
      // Get rid of matching param
      var newLine = toks.shift();

      var src = '';
      while (toks.length > 0) {
        var param = toks.shift();

        if (args[param]) {
          newLine += ' ' + toks.splice(0, args[param]).join(' ');
        } else {
          src += param + ' ' + toks.join(' ');
          break;
        }
      }

      if (!maps) {
        //src = getAbsoluteURL(src);
        if (src.search(/^[a-z]*:\/\//) == -1) {
          if (src.substr(0,1) == '/') {
            src = window.janus.currenturl() + src;
          } else {
            src = basePath + src;
          }
        }

        if (mapHashCache[src]) {
          newLine += ' /ipfs/' + mapHashCache[src];
          lines[index] = newLine;
          uploadMaps(cb);
          return;
        }

        getRemoteBlob(src, function(err, blob) {
          if (err) {
            lines[index] = '#' + lines[index];
            uploadMaps(cb);
            return;
          }

          uploadFile(blob, function(data) {
            mapHashCache[src] = data.hash;
            newLine += ' /ipfs/' + data.hash;
            lines[index] = newLine;
            uploadMaps(cb);
          })
        })
      } else {
        // Instead, upload our locally selected textures files ^-^
      }
    }

    uploadMaps(function() {
      console.log('Done with lines!');
      cb(lines.join('\n'));
    });
  }

  function getRemote(url, cb) {
    $.get(url, cb);
  }

  function getRemoteBlob(url, cb) {
    $.ajax({
      url : url,
      type: 'get',
      dataType: 'binary',

      xhr: function() {
        var xhr = new XMLHttpRequest();
        xhr.responseType = 'arraybuffer';
        return xhr;
      },

      success: function(data, status, xhr) {
        var blob = new Blob([data], {type: xhr.getResponseHeader('Content-Type')});
        cb(null, blob);
      },

      error: function(err, status, xhr) {
        cb(err, null);
      }
    })
  }

  function createNode(fileType, hash, parentHash, meta, cb) {
    if (!cb) {
      cb = meta;
      meta = null;
    }

    var newNode = {
      hash : hash,
      filename : 'New File',
      filetype : fileType
    };

    if (meta)
      newNode.meta = meta;

    var parent = inventoryJSON;

    if (parentHash)
      parent = fileCache[parentHash];

    if (!parent) {
      cb('Failed to find target location to add file', null);
    }

    parent.children.push(newNode);

    cacheFile(newNode, parent);
    saveInventory();
    cb(null, newNode);
  }

  function faksjf() {

    $.ajax({
      url : url,
      type: 'get',
      dataType: 'binary',

      xhr: function() {
        var xhr = new XMLHttpRequest();
        xhr.responseType = 'arraybuffer';
        return xhr;
      },

      success: function(data, status, xhr) {

      },
      // Cause screw error handling loooool
      error : function(e, status, xhr) { console.log('Error:', e, status, xhr) }
    })
  }

  function addNode(newNode, parentHash) {
    var parent = inventoryJSON;

    if (parentHash)
      parent = fileCache[parentHash];

    if (!parent) {
      cb('Failed to find target location to add file', null);
      return;
    }

    parent.children.push(newNode);

    cacheFile(newNode, parent);
    saveInventory();
    cb(null, newNode);
  }

  this.addFile = function(fileData, name, parentHash, cb) {
    cb = cb || function() {};

    // If parent hash is anything other than null, or a dir, error out
    if (parentHash && parentHash.substr(0, 4) != 'dir_') {
      cb('Invalid location', null);
      return;
    }

    var toks = fileData.name.split('.');

    var fileType = toks.pop();

    uploadFile(fileData, function(data) {
      var newNode = {
        hash : data.hash,
        filename : name,
        filetype : fileType
      };

      var parent = inventoryJSON;

      if (parentHash)
        parent = fileCache[parentHash];

      if (!parent) {
        cb('Failed to find target location to add file', null);
        return;
      }

      parent.children.push(newNode);

      cacheFile(newNode, parent);
      saveInventory(function() {
        cb(undefined, newNode);
      });
    });
  }

  this.addFolder = function(name, parentHash, cb) {
    cb = cb || function() {};

    // If parent hash is anything other than null, or a dir, error out
    if (parentHash && parentHash.substr(0, 4) != 'dir_') {
      cb('Invalid location', null);
      return;
    }

    var newNode = {
      hash : generateDirHash(),
      filename : name,
      filetype : 'dir',
      children : []
    };

    var parent = inventoryJSON;
    if (parentHash)
      parent = fileCache[parentHash];

    if (!parent) {
      cb('Failed to find target location to add folder', null);
      return;
    }

    parent.children.push(newNode);

    cacheFile(newNode, parent);

    saveInventory(function() {
      cb(undefined, newNode);
    });
  }

  this.moveFile = function(sourceHash, destHash, cb) {
    cb = cb || function() {};

    var file = fileCache[sourceHash];

    var oldParent = fileParentCache[sourceHash];
    var newParent;

    var newParent = inventoryJSON;
    if (destHash)
      newParent = fileCache[destHash];

    var i = oldParent.children.indexOf(file);

    if (i == -1) {
      cb('Failed to find source directory');
      return;
    }

    oldParent.children.splice(i,1);
    newParent.children.push(file);

    saveInventory(function() {
      cb();
    });
  }

  this.getFile = function(hash) {
    return fileCache[hash];
  }

  this.getFolder = function(hash) {
    return fileCache[hash];
  }

  this.deleteFile = function(hash, cb) {
    cb = cb || function() {};

    var file = fileCache[hash];
    var parent = fileParentCache[hash];

    if (!parent) {
      cb('Filed to find parent directory');
      return;
    }

    var i = parent.children.indexOf(file);

    if (i == -1) {
      cb('Failed to find file');
      return;
    }

    parent.children.splice(i,1);

    saveInventory(cb);
  }

  this.rename = function(hash, name) {
    var file = fileCache[hash];

    file.filename = name;

    saveInventory();
  }

  this.save = function(cb) {
    cb = cb || function() {};
    saveInventory(function(err) {
      cb(err)
    });
  }

  this.init = function() {
    loadInventory(function() {

      this.dispatch('ready');
    }.bind(this));
  }
})();

InventoryAPI.init()
