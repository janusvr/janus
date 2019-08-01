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
