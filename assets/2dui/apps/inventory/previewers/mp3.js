var id = params.hash;

var audio = new Audio();
audio.src = assetURL;
audio.controls = true;

$('.windowarea').append(audio);
