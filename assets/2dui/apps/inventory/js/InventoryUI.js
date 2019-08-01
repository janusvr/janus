
function FileEntry(hash, name, type) {
  return {
    hash : hash,
    filename : name,
    filetype : type
  }
}


function InventoryUI(options) {
  log('Starting inventory');

  this.rezzableTypes = {
    'jpg' : 'Image',
    'gif' : 'Image',
    'png' : 'Image',
    'obj' : 'Object',
    'fbx' : 'Object',
    'html' : 'Link',
    'dae' : 'Object',
    'mp3' : 'Sound',
    'wav' : 'Sound',
    'js' : 'Script',
  }

  this.options = options;
  this.domElement = options.target;
  this.domElement.addClass('dir');
  this.cache = {};

  // Deselect all if background clicked
  $('.assets-container').on('click', function(ev) {
    if (ev.currentTarget == ev.target) {
      this.deselectAll();
    }
  }.bind(this));
  // Click and dragging
  this.domElement.on('click', '.entry-display', function(ev) {
    var $this = $(ev.currentTarget);
  }.bind(this));


  var dragState = {
    el : null,
    time : 0,
    startX : 0,
    startY : 0,
    isDragging : false
  };

  $(document).on('mousedown', '.entry-display', function(ev) {
    var $this = $(ev.currentTarget);

    var entry = $this.data('entry');


    dragState = {
      el : $this.closest('li.entry'),
      proxy : null,
      time : new Date().getTime(),
      startX : ev.pageX,
      startY : ev.pageY,
      isDragging : false
    };
  }.bind(this));

  $(document).on('mouseup', function(ev) {
    if (dragState.el && dragState.lastTarget) {
      var src = dragState.el;
      var dst = dragState.lastTarget;

      if (dst.hasClass('dropinto')) {
        this.moveFile(src, dst);
      }
      else if (dst.hasClass('dropabove') || dst.hasClass('dropbelow')) {
        var dst = dst.parent().closest('.entry.dir, .assets-container');
        this.moveFile(src, dst);
      }
    }

    if (dragState.lastTarget) {
      dragState.lastTarget.removeClass('dropabove');
      dragState.lastTarget.removeClass('dropbelow');
      dragState.lastTarget.removeClass('dropinto');
    }

    if (dragState.proxy)
      dragState.proxy.remove();

    dragState.time = 0;
    dragState.el = null;
    dragState.proxy = null;
    dragState.isDragging = false;
    dragState.lastTarget = null;
  }.bind(this));

  $(document).on('mousemove', function(ev) {
    if (!dragState.el)
      return;

    if (!dragState.isDragging) {
      var diffX = Math.pow(ev.pageX - dragState.startX, 2);
      var diffY = Math.pow(ev.pageY - dragState.startY, 2);
      var diff = Math.sqrt(diffX + diffY);

      if (diff > 4) {
        dragState.isDragging = true;
        var proxy = dragState.el.children('.entry-display').clone();
        proxy.addClass('drag-proxy');
        dragState.proxy = proxy;
        $('body').append(proxy);
      } else {
        return;
      }
    }

    if (dragState.proxy) {
      dragState.proxy.css({
        left: ev.pageX + 20,
        top: ev.pageY - 10,
      });

      var target = $(ev.target).closest('li.entry');

      if (dragState.lastTarget && dragState.lastTarget != target) {
        dragState.lastTarget.removeClass('dropabove');
        dragState.lastTarget.removeClass('dropbelow');
        dragState.lastTarget.removeClass('dropinto');
      }

      dragState.lastTarget = null;
      // No recursion for you!
      if (!target ||
          !target.length ||
          target[0] == dragState.el[0] ||
          dragState.el.find(target).length)
        return;

      var proxyX = target.offset().left;
      var proxyY = target.offset().top + target.height() * 0.5;

      if (target.hasClass('dir') && ev.pageX < proxyX + 50) {
        target.addClass('dropinto');

      }
      else if (ev.pageY < proxyY) {
        target.addClass('dropabove');
      } else {
        target.addClass('dropbelow');
      }

      dragState.lastTarget = target;
    }

  }.bind(this));

  $(document).on('keydown', function(ev) {
    if (ev.which == 46) {
      this.deleteEntries($('.active').parent());
    }
  }.bind(this));
  /* ---------------------
      Cancel renaming if we click on anything except for
      the renaming input.
     --------------------- */
  $(document).on('click', function(ev) {
    if (!$(ev.target).parent().hasClass('renaming'))
      this.cancelRenamingFile();
  }.bind(this))

  /* ---------------------
      Handle keypresses for canceling/saving renaming
     --------------------- */
  this.domElement.on('keydown', '.renaming input', function(ev) {
    ev.stopPropagation();
    if (ev.which == 13) {
      this.finishRenameFile();
    } else if (ev.which == 27) {
      this.cancelRenamingFile();
    }
  }.bind(this));
  /* ---------------------
    Double click functionality
      For expanding folders, opening previews, and renaming
     --------------------- */
  var renameTimeout = null;
  this.domElement.on('click', '.entry-display', function(ev) {
    var $this = $(ev.currentTarget);

    var now = new Date().getTime();
    var timeDiff = now - $this.data('lastclick');

    if (timeDiff < 300) {
      var entry = $this.data('entry');

      if ($this.hasClass('renaming'))
        return;

      if (renameTimeout) {
        clearInterval(renameTimeout);
        renameTimeout = null;
      }
      if (entry.data('filetype') == 'dir') {
        this.toggleFolder(entry);
      } else {
        this.previewAsset(entry);
      }

      $this.data('lastclick', 0);
    } else if ($this.hasClass('active') && !$this.hasClass('renaming') && timeDiff > 500) {

      $this.data('lastclick', now);
      var entry = $this.data('entry');
      renameTimeout = setTimeout(function() {
        renameTimeout = null;
        this.startRenameFile(entry);
      }.bind(this), 300);


    } else {
      var entry = $this.data('entry');
      this.selectEntry(entry);
      $this.data('lastclick', now);
    }
  }.bind(this));

  // Expander mouse handling
  this.domElement.on('click', '.expander', function(ev) {
    var $this = $(ev.currentTarget);

    var folder = $this.data('entry');
    this.toggleFolder(folder);
  }.bind(this))


  this.inventory = InventoryAPI.getInventory();
  this.parseInventory(this.inventory);
  this.initUploader();
  //this.refreshFolder();

  $('.action-createfolder').on('click', function() {
    if ($(this).hasClass('disabled'))
      return;
    this.addFolder('New Folder', this.getActiveFolder());
  }.bind(this));

  $('.action-trashselected').on('click', function(ev) {
    if ($(ev.target).hasClass('disabled'))
      return;

    this.deleteEntries($('.active').parent());
  }.bind(this))

  $('.action-rezselected').on('click', function() {
    if ($(this).hasClass('disabled'))
      return;
    // Rezzes ALL selected objects
    // ...maybe not a good idea. Totally not abusable...
    this.rezEntries($('.active').parent());
  }.bind(this))

  $('.action-copyselected').on('click', function() {
    if ($(this).hasClass('disabled'))
      return;

    var selected = janus.selected_asset;

    if (!selected)
      return;

    var parentEl = this.getActiveFolder();
    var hash = parentEl.data('hash')

    InventoryAPI.addFromAssetObject(selected, hash, function(err, childNode) {
      this.addToFolder(parentEl, childNode, true);
      this.orderFolder(parentEl);
    }.bind(this));
  }.bind(this));

  $('.action-getselectedurl').on('click', function() {
    var active = $('.active').parent();
    var hash = active.data('hash');
    var file = InventoryAPI.getFile(hash);
	var progbar = parent.document.getElementById("progressbar");
	progbar.value = 'http://ipfs.io/ipfs/' + hash + '?.' + file.filetype;
	progbar.focus();
	parent.shownotification('URL copied to your address bar.','notifications/call.png','null','#323232')
  }.bind(this))

  this.spawnRandom = function() {
    $.get('http://ipfs.pics/random', function(data, a, xhr) {
      try {
        var i = data.search('img');
        data = data.substr(i+100);
        i = data.search('img');
        var src = data.substr(i+9, 69) + '?.jpg';

        var plr = janus.playerlist[0];
        //alert(JSON.stringify(plr));

        var rezDist = 3;
        var rezHeight = 1.5;
        //parent.window.janus.createasset('Image', {id:src, src:src});

        var rezData = {
          src:src,
          pos:
            (plr.pos.x + plr.zdir.x * rezDist) + ' ' +
            (plr.pos.y + plr.zdir.y * rezDist + rezHeight) + ' ' +
            (plr.pos.z + plr.zdir.z * rezDist) + ' '
        }

        rezData.xdir = [plr.xdir.x, plr.xdir.y, plr.xdir.z].join(' ');
        rezData.ydir = [plr.ydir.x, plr.ydir.y, plr.ydir.z].join(' ');
        rezData.zdir = [plr.zdir.x*-1, plr.zdir.y*-1, plr.zdir.z*-1].join(' ');

        parent.window.janus.createobject('Image', rezData);
      } catch (e) { parent.logToConsole(e) }
    })
  }

  $('.action-rezrandom').on('click', function() {
    this.spawnRandom();
  }.bind(this));
}

InventoryUI.prototype.rezEntries = function(entries) {
  entries.each(function(i,e) {
    this.rezEntry($(e));
  }.bind(this));
}

InventoryUI.prototype.rezEntry = function(entry) {
  var hash = entry.data('hash');

  var fileData = InventoryAPI.getFile(hash);
  var assetType;
  if (!(assetType = this.getAssetType(entry))) {
    //alert('Filetype ' + fileData.filetype + ' is not currently rezzable :(');
    return;
  }

  var janus = parent.window.janus;

  var url = 'http://ipfs.io/ipfs/' + hash + '?.' + fileData.filetype;

  if (assetType == 'Link') {
    window.janus.launchurl(url,0);
  }


  //alert(JSON.stringify(janus.playerlist[0].pos));
  try {
    var plr = janus.playerlist[0];
    //alert(JSON.stringify(plr));

    var rezDist = 3;
    var rezHeight = 1.5;
    var assetData = {id:hash, src:url};

    /*if (fileData.fileType == 'html') {
      delete assetData.src;
      assetData.url = url;

      scale="2.29 2.29 1" col="#c0c040" url="?x=4&y=6" auto_load="true" draw_glow="false" draw_text="false" />
    }*/

    if (fileData.meta && fileData.meta.mtl)
      assetData.mtl = 'http://ipfs.io/ipfs/' + fileData.meta.mtl;

    if (fileData.fileType != 'html') {
      janus.createasset(assetType, assetData);
    }

    if (assetType == 'Script')
      return;

    var rezData = {
      id:hash,
      pos:
        (plr.pos.x + plr.zdir.x * rezDist) + ' ' +
        (plr.pos.y + plr.zdir.y * rezDist + rezHeight) + ' ' +
        (plr.pos.z + plr.zdir.z * rezDist) + ' '
    }

    if (assetType == 'Link') {
      //alert('?');
      rezData.url = url;
      delete rezData.id;
      rezData.scale = [1,1.75,1].join(' ');
    }

    if (['Image', 'Link'].indexOf(assetType) != -1) {
      rezData.xdir = [plr.xdir.x, plr.xdir.y, plr.xdir.z].join(' ');
      rezData.ydir = [plr.ydir.x, plr.ydir.y, plr.ydir.z].join(' ');
      rezData.zdir = [plr.zdir.x*-1, plr.zdir.y*-1, plr.zdir.z*-1].join(' ');
    }

    janus.createobject(assetType, rezData);

  //alert(o);
  }
  catch (e) {
    parent.logToConsole(':(' + e + ' ' + JSON.stringify(plr));
  }
  //fileData.filetype
}

InventoryUI.prototype.initUploader = function() {

  var fileSel = $('.file-input');
  var uploadBtn = $('.action-selectfile');

  uploadBtn.on('click', function() {
    fileSel.click();
  });

  fileSel.on('change', function(ev) {

    var parentEl = this.getActiveFolder();
    var parentHash = parentEl.data('hash');
    var fileData = ev.target.files[0];

    var toks = fileData.name.split('.');

    var fileType = '';
    var fileName = '';

    if (toks.length == 1) {
      fileName = toks[0];
    } else {
      fileType = toks.pop();

      if (fileType == 'gz')
        fileType = toks.pop() + '.gz';

      fileName = toks.join('.');
    }

    InventoryAPI.addFile(fileData, fileName, parentHash, function(err, childNode) {
      this.addToFolder(parentEl, childNode);
      this.orderFolder(parentEl);
    }.bind(this));

    fileSel.wrap('<form/>').closest('form').get(0).reset();
    fileSel.unwrap();

  }.bind(this));

}


InventoryUI.prototype.addFolder = function(name, folderEl) {
  var hash = folderEl.data('hash');

  InventoryAPI.addFolder(name, hash, function(err, node) {
    this.addToFolder(folderEl, node);
    this.orderFolder(folderEl);
  }.bind(this));
}

InventoryUI.prototype.getActiveFolder = function() {
  var active = this.domElement.find('.active').eq(0);

  if (active.length == 0)
    return this.domElement;

  if (active.hasClass('dir'))
    return active;

  return active.closest('.dir');
}

InventoryUI.prototype.previewAsset = function(entry) {

  var title = 'Asset Previewer<div style="font-size: 8px; position: absolute; opacity: 0">' + entry.data('hash') + '<div>';

  var iframe = parent.postMessage({
    cmd : 'toggleWindow',
    callerid : 'null',
    width : 400,
    height : 400,
    title : title,
    page : 'apps/inventory/preview.html',//?hash=' + entry.data('hash') + '&filetype=' + entry.data('filetype'),
    spawnx : '20vw',
    spawny : '15vh'
  }, '*');

  setTimeout(function() {
    var wind = parent.document.getElementById("window_" + title.replace(/ /g,'&nbsp;') + "_windowarea");
    var frame = wind.childNodes[0];

    frame.contentWindow.addEventListener('load', function() {
      console.log('sending data');
      frame.contentWindow.postMessage({
        cmd: 'asset',
        data : InventoryAPI.getFile(entry.data('hash'))
      }, '*')
    });
  });
}

InventoryUI.prototype.moveFile = function(srcNode, dstNode) {
  dstNode.children('ul').append(srcNode);
  InventoryAPI.moveFile(srcNode.data('hash'), dstNode.data('hash'));
  this.orderFolder(dstNode);
}

InventoryUI.prototype.startRenameFile = function(entry) {
  this.cancelRenamingFile();

  var display = entry.children('.entry-display');
  display.addClass('renaming');
  var input = $('<input />');
  input.val(display.children('.filename').text());
  display.append(input);
  input.focus();
  input.select();


  input.on('blur', function onBlur() {
    $(this).off('blur', onBlur);
    this.cancelRenamingFile();
  }.bind(this));
}

InventoryUI.prototype.finishRenameFile = function() {
  var renaming = $('.renaming');
  if (renaming.length == 0)
    return;

  var name = renaming.find('input').val()
  renaming.children('.filename').text(name);

  var entry = renaming.parent('li');
  var hash = entry.data('hash');

  InventoryAPI.rename(hash, name);

  this.orderFolder(entry.parent().closest('.dir'));

  this.cancelRenamingFile();
}

InventoryUI.prototype.cancelRenamingFile = function() {
  var renaming = $('.renaming');

  if (renaming.length == 0)
    return;

  renaming.removeClass('renaming');
  renaming.find('input').remove();
}

InventoryUI.prototype.deleteEntries = function(entries) {
  /* TODO: this will likely cause problems if you have a parent
    and child selected and the parent is deleted before the child. I dunno.
    YOLO, I'll fix it later when multi select is possible */

  entries.each(function() {

    var $this = $(this);
    var hash = $this.data('hash');

    InventoryAPI.deleteFile(hash);
  });

  // UI Update :D
  entries.remove();

  $('.action-trashselected').addClass('disabled');
}

InventoryUI.prototype.getAssetType = function(entry) {
  var hash = entry.data('hash');
  var file = InventoryAPI.getFile(hash);

  var fileType = file.filetype.split('.').shift();

  return this.rezzableTypes[fileType];
}

InventoryUI.prototype.selectEntry = function(entry) {

  this.domElement.find('.active').removeClass('active');
  entry.children('.entry-display').addClass('active');

  if (entry.data('filetype') != 'dir')
    $('.url-output').val(this.getAssetURL(entry));


  /* TODO make this emit a "selectchange" event and handle this elswhere */
  var hash = entry.data('hash');
  var file = InventoryAPI.getFile(hash);


  if (!file || !this.getAssetType(entry)) {

    $('.action-rezselected').addClass('disabled');
  } else {
    $('.action-rezselected').removeClass('disabled');
  }

  $('.action-trashselected').removeClass('disabled');
}

InventoryUI.prototype.deselectAll = function() {
  this.domElement.find('.active').removeClass('active');
  $('.action-trashselected').addClass('disabled');
}

InventoryUI.prototype.getAssetURL = function(entry) {
  var hash = entry.data('hash');
  return 'http://ipfs.io/ipfs/' + hash;
  //return this.baseurl + 'asset/get/' + hash;
}

InventoryUI.prototype.toggleFolder = function(folder) {
  if (folder.hasClass('expanded')) {
    this.collapseFolder(folder);
  } else {
    this.expandFolder(folder);
  }
}

InventoryUI.prototype.expandFolder = function(folder) {
  folder.addClass('expanded');
}

InventoryUI.prototype.collapseFolder = function(folder) {
  folder.removeClass('expanded');
}

InventoryUI.prototype.orderFolder = function(folder) {
  var list = folder.children('ul');
  var entries = list.children('.entry');
  var textA = '';
  var textB = '';
  var aDir = false;
  var bDir = false;


  entries.sort(function(a,b) {
    aDir = $(a).hasClass('dir');
    bDir = $(b).hasClass('dir');

    if (aDir != bDir) {
      if (aDir)
        return -1;
      else
        return 1;
    }

    textA = $(a)
      .children('.entry-display')
      .children('.filename')
      .text()
      .toLowerCase();
    textB = $(b)
      .children('.entry-display')
      .children('.filename')
      .text()
      .toLowerCase();

    return textA > textB ? 1 : -1;
  })

  entries.detach().appendTo(list);
}

InventoryUI.prototype.addToFolder = function(folder, node, highlight) {
  // No sublist, create one
  if (folder.children('ul').length == 0) {
    folder.append('<ul></ul>');
  }

  var list = folder.children('ul');

  var children = node.children;
  var entry = $('<li></li>');

  entry.addClass('entry');

  entry.data('filetype', node.filetype);
  entry.data('filename', node.filename);
  entry.data('hash', node.hash);

  var entryDisplay = $('<div/>');
  entryDisplay.addClass('entry-display');
  entryDisplay.data('entry', entry);

  entry.append(entryDisplay);

  var icon = $('<span/>');
  icon.addClass('icon');

  if (node.filetype) {
    var type = node.filetype;

    if (type.substr(type.length-3,3) == '.gz') {
      type = type.substr(0,type.length-3);
    }

    icon.addClass('icon-' + type);
  }

  entryDisplay.append('<span class="filename">' + node.filename + '</span>');
  entryDisplay.prepend(icon);


  if (node.filetype == 'dir') {
    var expander = $('<div/>');
    expander.data('entry', entry);
    expander.addClass('icon expander');
    entry.prepend(expander);
    entry.addClass('dir');

    if (entry.children('ul').length == 0) {
      entry.append('<ul></ul>');
    }
    /*if (Math.random() > 0.6) {
      var target = li;
      this.refreshFolder(target, function() {
        this.expandFolder(target);
      }.bind(this));
    }*/
    //li.addClass('expanded');

    if (children) {
      this.parseInventory(node, entry);
    }
  } else {
    entry.addClass('file');

  }

  list.append(entry);

  if (highlight) {
    this.deselectAll();
    this.selectEntry(entry);
  }
}

InventoryUI.prototype.parseInventory = function(nodes, folder) {

  if (!folder)
    folder = this.domElement;

  for (var i in nodes.children) {
    this.addToFolder(folder, nodes.children[i]);
  }

  this.orderFolder(folder);
}

var callbacks = {};
function onJSON(id, cb) {
  callbacks[id] = cb;
}

function getJSON(url, cb) {
  var id = Math.random();
  callbacks[id] = cb;

  if (url.search(/\?/) == -1) {
    url += '?rid=' + id;
  } else {
    url += '&rid=' + id;
  }
  var script = $("<script />", {
    src: url,
    type: "application/javascript"
  });

  $('head').append(script);
}

function consumeJSON(id, json) {
  if (callbacks[id])
    callbacks[id](json);
}


var dropdownMenu = $('.dropdown-menu');

parent.document.addEventListener('mouseup', function(ev) {
  dropdownMenu.removeClass('active');
});

document.addEventListener('mouseup', function(ev) {
  dropdownMenu.removeClass('active');
});

dropdownMenu.click(function(ev) {
  $(this).toggleClass('active');
});

InventoryAPI.on('ready', function() {
  $('.windowcontainer')
    .addClass('connected');

  assMan = new InventoryUI({
    target: $('.assets-container')
  });

  InventoryAPI.on('syncstart', function() {
    $('.windowcontainer')
      .addClass('syncing');
  })

  InventoryAPI.on('syncend', function() {
    $('.windowcontainer')
      .removeClass('syncing');
  })
});
