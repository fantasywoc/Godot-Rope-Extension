extends Node

var rope_simulator: ExampleClass
var draw_node: Node2D
var screen_center: Vector2

func _ready():
	# Create a Node2D for drawing
	draw_node = Node2D.new()
	add_child(draw_node)
	
	# Connect the draw function
	draw_node.draw.connect(_on_draw)
	
	# Get screen center
	screen_center = get_viewport().get_visible_rect().size / 2
	
	# Create rope simulator
	rope_simulator = ExampleClass.new()
	rope_simulator.set_node_count(20)        # 增加到20个节点
	rope_simulator.set_rope_length(500.0)    # 减少到300像素
	rope_simulator.set_gravity(Vector2(0, 98))
	rope_simulator.reset_rope()
	rope_simulator.setNodeLocked(0, true)
	rope_simulator.setNodeLocked(10, true)
	
  

func _process(delta):
	rope_simulator.update_physics(delta)
	draw_node.queue_redraw()  # Trigger redraw

func _on_draw():
	var positions = rope_simulator.get_node_positions()
	
	# Draw rope segments with center offset
	for i in range(positions.size() - 1):
		var start_pos = positions[i] + screen_center
		var end_pos = positions[i + 1] + screen_center
		draw_node.draw_line(start_pos, end_pos, Color.BROWN, 3.0)
	
	# Draw nodes with center offset
	for pos in positions:
		draw_node.draw_circle(pos + screen_center, 4.0, Color.RED)

func _input(event):
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		var mouse_pos = draw_node.get_global_mouse_position()
		var positions = rope_simulator.get_node_positions()
		if positions.size() > 0:
			# 计算相对于绳子中心的鼠标位置
			var last_node_pos = positions[-1] + screen_center
			var force = (mouse_pos - last_node_pos) * 50.0
			rope_simulator.apply_force(positions.size() - 1, force)
