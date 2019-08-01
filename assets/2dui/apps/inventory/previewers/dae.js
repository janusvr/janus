function onModelViewerReady(view) {
  include('lib/three/loaders/ColladaLoader.js').onload = function() {
    var loader = new THREE.ColladaLoader();
    loader.options.convertUpAxis = true;

    loader.load(assetURL, function(dae) {
      view.addAsset(dae.scene, bytes);
    }, function(progress) {
      bytes = progress.loaded
      //bytes = xhr.loaded;
    });
  }
}

include('three_renderer.js');
