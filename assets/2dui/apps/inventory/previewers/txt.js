$.get(assetURL, function(data) {
  var pre = $('<pre/>');
  $('.windowarea').append(pre.append(data));

  pre.css({
    'white-space': 'pre-wrap',
    padding: 0,
    margin: 0,
    width: '100%',
    height: '100%',
    color: '#2eb954'
  })
});
