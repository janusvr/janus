function onModelViewerReady(view) {
  try {
    asdf(view)
  } catch (e) {
    alert(e);
  }
}

function asdf(view) {
  include('lib/three/loaders/OBJLoader.js', function() {

    if (assetData.meta.mtl) {
      include('lib/three/loaders/MTLLoader.js', function() {
        loadMTL();
      })
    } else {
      loadOBJ();
    }
  });

  function loadMTL() {
    var mtlLoader = new THREE.MTLLoader();
		//mtlLoader.setPath( 'obj/male02/' );
    var mtlPath = baseurl + assetData.meta.mtl;
		mtlLoader.load(mtlPath, function(materials) {
      loadOBJ(materials);
    });
  }

  function loadOBJ(mtl) {
    var loader = new THREE.OBJLoader();

    if (mtl)
      loader.setMaterials(mtl);

    var bytes = 0;
    loader.load(assetURL, function(obj, a, b,c) {
      view.addAsset(obj, bytes);
    }, function(progress) {
      bytes = progress.loaded
      //bytes = xhr.loaded;
    });
  }
}

include('three_renderer.js');
