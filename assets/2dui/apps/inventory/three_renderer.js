var modelView;

// Lol...At least these are local files.
include('lib/three/three.min.js').onload = function() {
  include('lib/three/OrbitControls.js').onload = function() {
    include('lib/three/CanvasRenderer.js').onload = function() {
      include('lib/three/Projector.js').onload = function() {
        modelView = new ModelView();
      }
    }
  }
}

function ModelView() {
  this.lastRender = 0;


  try {
    this.renderer = new THREE.WebGLRenderer();
  } catch (e) {
    this.renderer = new THREE.CanvasRenderer();
  }

  var domEl = this.renderer.domElement;
  $('.windowarea').append(domEl);

  this.renderer.setSize(window.innerWidth, window.innerHeight);
  domEl.style.width = '100%';
  domEl.style.height = '100%';
  domEl.style.position = 'absolute';
  domEl.style.top = '0';

  this.scene = new THREE.Scene();
  this.camera = new THREE.PerspectiveCamera(75, window.innerWidth/window.innerHeight, 0.1, 1000);
  this.setupLights();

  this.camera.position.set(0,0,0.1);

  if (typeof onModelViewerReady == 'function')
    onModelViewerReady(this);

  this.zoom = 0;
  this.focus = new THREE.Vector3(0,0,0);

  this.controls = new THREE.OrbitControls( this.camera, this.renderer.domElement );
	this.controls.enableDamping = true;
	this.controls.dampingFactor = 0.9;
	this.controls.enableZoom = true;
	this.controls.enablePan = true;

  window.addEventListener('resize', function() {
    this.renderer.setSize(window.innerWidth, window.innerHeight);

    this.camera.aspect = window.innerWidth/window.innerHeight;
    this.camera.updateProjectionMatrix();
    this.render(true);
  }.bind(this));

  this.frame();
  this.frame = this.frame.bind(this);
  this.controls.addEventListener( 'change', this.frame ); // add this only if there is no animation loop (requestAnimationFrame)
}

ModelView.prototype.setupLights = function() {
  this.light = new THREE.DirectionalLight(0xFFFFFF);//, 1, 100);
  this.light.position.set(10, 5, 0);

  this.scene.add(this.light);

  this.ambient = new THREE.AmbientLight(0x404040);
  this.scene.add(this.ambient);
}


ModelView.prototype.addMesh = function(mesh) {
  this.scene.add(mesh);
  this.render(true);
}



ModelView.prototype.setZoom = function(zoom) {
  this.zoom = zoom;
  this.controls.object.position.z = zoom;
  this.controls.update();
  this.render(true);
}

/*
* I suspect we'll get some funky results with models
* offset from their origin. TODO check this later.
*/
ModelView.prototype.setFocus = function(focus) {
  this.controls.target.copy(focus);
  this.controls.object.position.y = focus.y;
  this.render(true);
}

ModelView.prototype.frame = function() {
  this.render(true);
}

ModelView.prototype.render = function(force) {
  var now = new Date().getTime();
  if (now - this.lastRender < 25 && !force)
    return;

  this.lastRender = now;
  this.renderer.render(this.scene, this.camera);
}

ModelView.prototype.addAsset = function(obj, bytes) {
  var view = this;
  var filesize = bytes ? Math.floor(bytes / 1024) + 'kb' : 'Unknown';
  var vertices = 0;
  var polys = 0;
  var meshCount = 0;


  obj.traverse(function(child) {
    if (child.material) {
      child.material.overdraw = 0.5;
      //child.material.shading = THREE.SmoothShading;
    }

    if (child.geometry) {
      child.geometry.computeFaceNormals();
      //child.geometry.computeVertexNormals();
      if (child.geometry instanceof THREE.BufferGeometry) {
        polys += child.geometry.attributes.position.count;
        vertices += child.geometry.attributes.position.count * 3;
      } else {
        polys += child.geometry.faces.length;
        vertices += child.geometry.vertices.length;

      }
        meshCount++;
    }
  });

  view.addMesh(obj);

  var bbox = new THREE.Box3().setFromObject(obj);
  var size = new THREE.Vector3(
    bbox.max.x - bbox.min.x,
    bbox.max.y - bbox.min.y,
    bbox.max.z - bbox.min.z
  );

  var biggest = Math.max(size.x, Math.max(size.y, size.z));
  //console.log(biggest);
  view.setFocus(bbox.center());
  view.setZoom(biggest);

  displayStats(vertices, polys, filesize, meshCount);
  displayMenu();
}

function displayStats(verticeCount, polyCount, bytes, meshCount) {
  var stats = $('<div/>');
  stats.css({
    position: 'absolute',
    left: 10,
    top: 10,
  });

  var verts = $('<div/>');
  var faces = $('<div/>');
  var size = $('<div/>');

  verts.text('Vertices: ' + verticeCount)
  faces.text('Polygons: ' + polyCount)
  size.text('Filesize: ' + bytes)
  //size.text('Vertices: ' + bytes)

  $(verts).add(faces).add(size).css({
    color: 'white',
    fontFamily: 'Helvetica'
  })
  .appendTo(stats);

  $('.windowarea').append(stats);
}

function displayMenu() {
  var menu = $('<div/>');
  menu.css({
    position: 'absolute',
    left: 0,
    right: 0,
    bottom: 0,
    height: 40,
    'z-index' : 1000,
    //background: '#555'
  });


  $('.windowarea').append(menu);
}
