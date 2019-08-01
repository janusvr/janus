(function() {
  var id = params.hash;
  var intervalID;
  var img = new Image();
  img.onload = function() {

  }

  img.onerror = function() {
    clearInterval(intervalID);
  }

  img.src = assetURL;


  // A bit of a hack to autosize for gifs as soon as they begin to load
  intervalID = setInterval(function() {
    if (!img.width)
      return;

    setWindowSize(img.width + 'px', img.height + 'px');

    img.style.width = '100%';
    img.style.height = '100%';

    clearInterval(intervalID);
  }, 16)

  $('.windowarea')
    .append(img)

})();
