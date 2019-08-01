function Dispatcher() {
  var listeners = [];

  this.on = function(ev, cb) {
    if (!listeners[ev])
      listeners[ev] = [];

    listeners[ev].push(cb);
  }

  this.off = function(ev, cb) {
    if (!listeners[ev])
      return;

    var i = listeners[ev].indexOf(cb);

    if (i == -1)
      return;

    listeners[ev].splice(i,1);
  }

  this.dispatch = function(ev, data) {
    if (!listeners[ev])
      return;

    for (var i = 0, il = listeners[ev].length; i < il; ++i) {
      listeners[ev][i](data);
    }
  }
};

var gDispatcher = new Dispatcher();

var winid = null;
window.params = {};

function setWindowSize(width, height) {
  if (!winid)
    return;

  parent.postMessage({
    cmd: 'setWindowSize',
    width: width,
    height: height,
    winid : winid
  }, '*');
}

function setCookie(cname, cvalue, exdays) {
  var d = new Date();
  d.setTime(d.getTime() + (exdays*24*60*60*1000));
  var expires = "expires="+d.toUTCString();
  document.cookie = cname + "=" + cvalue + "; " + expires;
}

function getCookie(cname) {
  var name = cname + "=";
  var ca = document.cookie.split(';');
  for(var i = 0; i < ca.length; i++) {
    var c = ca[i];
    while (c.charAt(0) == ' ') {
      c = c.substring(1);
    }
    if (c.indexOf(name) == 0) {
      return c.substring(name.length, c.length);
    }
  }
  return "";
}

function include(src, cb) {
  cb = cb || function() {};
  if (src.substr(0,1) == '/')
    src = params.rootdir + src;

  src += '?r=' + Math.random();
  var s = document.createElement('script');
  s.async = true;
  s.src = src;

  s.addEventListener('load', function() {
    cb();
  }, false);

  document.head.appendChild(s);

  return s;
}

(function() {
  var winReady = false;
  var domReady = false;
  var isInited = false;

  var listeners = [];
  window.onJanusWinReady = function(cb) {
    if (!winReady || !domReady)
      listeners.push(cb);
    else cb();
  }


  window.addEventListener('message', function(ev) {
    if (ev.data.cmd == 'wininit') {
      winid = ev.data.winid;
      winReady = true;
      window.rootdir = ev.data.rootdir;


      loadCheck();
    }
  }, false);

  window.addEventListener('load', function() {
    domReady = true;

    loadCheck();
  }, false);

  function readParams() {
    var query = location.href.split('?').pop();

    var toks = query.split('&');

    for (var i in toks) {
      var pair = toks[i].split('=');

      var val = pair.pop();
      var key = pair.pop();

      params[key] = val;

    }
  }

  function loadCheck() {

    if (!winReady || !domReady)
      return;

    for (var i in listeners) {

      listeners[i]();
    }

    listeners = [];
  }

  readParams();
})()
