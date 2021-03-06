require 'vizkit'
viz = Vizkit.default_loader.OctomapWrapperVisualization

# create an instance of the base type Vectro3d
position = slam::octomap_wrapper::OctomapWrapper.new

# a global index
i = 0.0
# initialize timer
timer = Qt::Timer.new
timer.connect(SIGNAL('timeout()')) do
    i += 1.0
    j = i / 200.0
    position.x = Math::cos(j) * j
    position.y = Math::sin(j) * j
    position.z = 0.5
    viz.updateData(position)
end

# start the timer with a timeout of 10 ms
timer.start(10)
Vizkit.exec
