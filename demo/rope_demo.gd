extends Node
var locked_nodes: Array = []  # 记录锁定的节点索引

var rope_simulator: ExampleClass
var draw_node: Node2D
var screen_center: Vector2

var debug_enabled: bool = false
var frame_counter: int = 0

# 拖拽相关变量
var is_dragging: bool = false
var dragged_node_index: int = -1
var drag_offset: Vector2
var drag_threshold: float = 20.0  #检测拖拽的阈值

# 双击检测相关变量
var last_click_time: float = 0.0
var last_clicked_node: int = -1
var double_click_threshold: float = 0.5  # 双击时间0.5秒

# 单击固定功能相关变量
var click_timer: Timer
var pending_click_node: int = -1
var single_click_delay: float = 0.3 

# 缓存变量，避免重复计算
var cached_positions: Array
var positions_dirty: bool = true

# 在变量声明区域修改
var base_rope_length: float = 500.0  # 基准绳长
var desired_display_length: float = 600.0  # 期望的显示长度（像素）

var scale_factor: float = 5.0  # 每一节绳子的像素长度
var rope_node_count:int =10; #绳子节点数
var gravity_multiplier = 3.0;   #重力加速度
var ropeColor = Color.DARK_GOLDENROD
var lineWith = 7.0;
var nomal_node_size = lineWith ;

@onready var rope_node: Marker2D = $".."

func _ready():
	# 从marker2d 导出参数同步数据
	scale_factor = rope_node.scale_factor
	rope_node_count = rope_node.rope_node_count
	gravity_multiplier = rope_node.gravity_multiplier
	ropeColor = rope_node.ropeColor
	lineWith = rope_node.lineWith

	# 绘制节点
	draw_node = Node2D.new()
	add_child(draw_node)
	draw_node.draw.connect(_on_draw)
	
	# 单击计时器
	click_timer = Timer.new()
	click_timer.wait_time = single_click_delay
	click_timer.one_shot = true
	click_timer.timeout.connect(_on_single_click_confirmed)
	add_child(click_timer)
	
	#改用父节点位置，避免在场景中多次实例化时，实际渲染位置相同
	screen_center = rope_node.global_position
	
	# 创建并配置绳索模拟器
	rope_simulator = ExampleClass.new()
	
	# 批量设置参数，避免多次初始化
	setup_rope_parameters()

	
	# 等待物理稳定后锁定节点
	await get_tree().process_frame
	setup_constraints()
	
	if debug_enabled:
		print("绳索初始化完成 - 节点数: %d, 物理长度: %.1f, 显示长度: %.1f像素" % [rope_simulator.get_node_count(), rope_simulator.get_rope_length(), base_rope_length])



# 在 setup_rope_parameters 函数中添加弹性设置
func setup_rope_parameters():
	rope_simulator.set_node_count(rope_node_count)
	rope_simulator.set_rope_length(base_rope_length)  
	rope_simulator.set_gravity_multiplier(gravity_multiplier) #使用重力倍率
	

	rope_simulator.setElasticityPreset("rigid")   #使用预设参数配置
	#自定义参数配置
	#var gentle_rigid_params = {
		#"stiffness": 0.4,              # 较低刚度避免过度修正
		#"damping": 0.98,              # 高阻尼
		#"iterations": 5,             # 更多迭代补偿较低刚度
		#"constraint_strength": 0.2,    # 较强约束但不是最强
		#"stretch_resistance": 0.3,     # 适中阻力
		#"compression_resistance": 1.6   # 稍强压缩阻力
	#}
	#rope_simulator.setAdvancedElasticity(gentle_rigid_params)

# 添加按键和鼠标控制
func _input(event):

	if event is InputEventKey and event.pressed:
		match event.keycode:
			KEY_F1:  
				rope_simulator.setElasticityPreset("rigid")
				print("切换到刚性模式")
			KEY_F2:  
				rope_simulator.setElasticityPreset("elastic")
				print("切换到弹性模式")
			KEY_F3:  
				
				rope_simulator.setElasticityPreset("soft")  
				print("切换到柔软模式")
			KEY_F4:  # 弹跳模式
				rope_simulator.setElasticityPreset("bouncy")
				print("切换到弹跳模式")
			KEY_Q:  # 增加刚度
				rope_simulator.adjustStiffness(0.1)
				print("增加刚度")
			KEY_A:  # 减少刚度
				rope_simulator.adjustStiffness(-0.1)
				print("减少刚度")
			KEY_W:  # 增加阻尼
				rope_simulator.adjustDamping(0.01)
				print("增加阻尼")
			KEY_S:  # 减少阻尼
				rope_simulator.adjustDamping(-0.01)
				print("减少阻尼")
			KEY_E:  # 增加约束强度
				rope_simulator.adjustConstraintStrength(0.1)
				print("增加约束强度")

			KEY_P:  # 获取当前弹性参数
				var params = rope_simulator.getElasticityParams()
				print("当前弹性参数: ", params)
			KEY_1: # 设置第5个节点到指定位置
				rope_simulator.set_node_position(5, Vector2(200, 300))
			KEY_2: # 向右移动第3个节点
				rope_simulator.move_node(3, Vector2(50, 0))
			KEY_3: # 获取第0个节点位置
				var pos = rope_simulator.get_node_position(0)
				print("Node 0 position: ", pos)
			KEY_4: # 整体向上移动
				rope_simulator.translate_all_nodes(Vector2(0, -20))
			KEY_5: # 移动中间几个节点
				rope_simulator.translate_nodes_range(2, 7, Vector2(10, -10))
			KEY_X:  # 移除中间节点
				remove_middle_node()
			KEY_R:  # 重置绳索
				reset_rope_system()
	
	# 处理鼠标滚轮输入 
	elif event is InputEventMouseButton:
		if event.pressed:
			if event.button_index == MOUSE_BUTTON_LEFT:
				handle_mouse_press(event.position)
			elif event.button_index == MOUSE_BUTTON_RIGHT:
				handle_right_click(event.position)
			# 新增：滚轮控制节点数量
			elif event.button_index == MOUSE_BUTTON_WHEEL_UP:
				handle_wheel_scroll(1)
			elif event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
				handle_wheel_scroll(-1)
		else:
			if event.button_index == MOUSE_BUTTON_LEFT:
				handle_mouse_release()
	
	# 处理鼠标移动（拖拽）
	elif event is InputEventMouseMotion:
		if is_dragging:
			update_drag(event.position)
	
	# 滚轮控制函数
func handle_wheel_scroll(direction: int):
		"""处理滚轮控制节点增减"""
		var input_map = Input
		
		var shift_pressed = input_map.is_action_pressed("ui_shift") or Input.is_key_pressed(KEY_SHIFT)
		var ctrl_pressed = input_map.is_action_pressed("ui_ctrl") or Input.is_key_pressed(KEY_CTRL)
		var alt_pressed = input_map.is_action_pressed("ui_alt") or Input.is_key_pressed(KEY_ALT)
		
		if ctrl_pressed:
			# Ctrl + 滚轮：调整段长度
			var length_delta = direction * 2.0
			adjust_segment_length(length_delta)
			print("调整段长度: ", length_delta, ", 当前段长度: ", scale_factor)
		elif shift_pressed:
			# Shift + 滚轮：批量增减节点，每次增减5个
			var node_delta = direction * 5
			adjust_node_count_batch(node_delta)
			print("批量调整节点数: ", node_delta, ", 当前节点数: ", rope_simulator.get_current_node_count())
		
		else:
			# 普通滚轮：单个节点增减
			adjust_node_count(direction)
			print("调整节点数: ", direction, ", 当前节点数: ", rope_simulator.get_current_node_count())
	
	#调整节点数量
func adjust_node_count(delta: int):
		
		var current_count = rope_simulator.get_current_node_count()
		var new_count = current_count + delta
		
		# 限制节点数量范围
		new_count = clamp(new_count, 2, 200)
		
		if delta > 0:
			# 增加节点
			for i in range(delta):
				if rope_simulator.get_current_node_count() < 200:
					rope_simulator.add_node_to_end()
		elif delta < 0:
			# 减少节点
			for i in range(abs(delta)):
				var count = rope_simulator.get_current_node_count()
				if count > 2:
					rope_simulator.remove_node(count - 1)  # 移除最后一个节点
					update_locked_nodes_after_removal(count - 1)


		positions_dirty = true
		rope_node_count = rope_simulator.get_current_node_count()
	
	# 批量调整节点数量
func adjust_node_count_batch(delta: int):
		"""批量调整节点数量"""
		if delta > 0:
			# 增加节点
			rope_simulator.add_nodes_to_end(delta)
		elif delta < 0:
			# 减少节点
			var current_count = rope_simulator.get_current_node_count()
			var remove_count = min(abs(delta), current_count - 2)  # 至少保留2个节点
			
			for i in range(remove_count):
				var count = rope_simulator.get_current_node_count()
				if count > 2:
					rope_simulator.remove_node(count - 1)
					update_locked_nodes_after_removal(count - 1)
		
		positions_dirty = true
		rope_node_count = rope_simulator.get_current_node_count()
	
# 调整绳段长度函数
func adjust_segment_length(delta: float):
	scale_factor = clamp(scale_factor + delta, 1.0, 20.0)
	positions_dirty = true



# 通过段长度设置绳子函数
func setup_rope_by_segment_length():

	scale_factor = 5.0
	rope_node_count = 100

	
	rope_simulator.set_node_count(rope_node_count)
	rope_simulator.set_rope_length(base_rope_length)

	
	setup_constraints()
	positions_dirty = true
	
	# print("重置绳子参数 - 节点数: ", rope_node_count, ", 段长度: ", scale_factor, ", 重力: ", rope_gravity)

func handle_mouse_press(mouse_pos: Vector2):

	var clicked_node = get_clicked_node(mouse_pos)
	if clicked_node == -1:
		return
	
	var current_time = Time.get_ticks_msec() / 1000.0
	
	# 检查双击
	if (current_time - last_click_time) < double_click_threshold and clicked_node == last_clicked_node:
		# 双击检测成功，剪断绳子
		click_timer.stop()  # 停止单击计时器
		pending_click_node = -1
		cut_rope_from_node(clicked_node)
		return
	
	# 更新点击状态
	last_click_time = current_time
	last_clicked_node = clicked_node
	
	# 立即开始拖拽（而不是等待单击确认）
	start_drag(mouse_pos)
	
	# 启动单击延迟计时器（用于在拖拽结束后处理固定功能）
	pending_click_node = clicked_node
	click_timer.start()

func handle_mouse_release():
	"""处理鼠标释放事件"""
	# 如果正在拖拽，停止拖拽
	if is_dragging:
		stop_drag()
		# 拖拽结束后，取消单击处理
		click_timer.stop()
		pending_click_node = -1

func start_drag(mouse_pos: Vector2):
	"""开始拖拽检测"""
	var positions = get_cached_positions()
	if positions.is_empty():
		return
	
	# 查找最近的节点
	var closest_index = -1
	var closest_distance = drag_threshold
	
	for i in range(positions.size()):
		var node_screen_pos = positions[i] * scale_factor + screen_center
		var distance = mouse_pos.distance_to(node_screen_pos)
		
		if distance < closest_distance:
			closest_distance = distance
			closest_index = i
	
	# 开始拖拽
	if closest_index != -1:
		is_dragging = true
		dragged_node_index = closest_index
		var node_screen_pos = positions[closest_index] * scale_factor + screen_center
		drag_offset = mouse_pos - node_screen_pos
		
		# 临时锁定被拖拽的节点，避免约束系统干扰
		rope_simulator.setNodeLocked(closest_index, true)
		
		if debug_enabled:
			print("开始拖拽节点: ", closest_index)

# 修改绘制函数，显示锁定状态
func _on_draw():
	var positions = get_cached_positions()
	if positions.is_empty():
		return
	
	# 绘制绳索段
	for i in range(positions.size() - 1):
		var start_pos = positions[i] * scale_factor + screen_center
		var end_pos = positions[i + 1] * scale_factor + screen_center
		draw_node.draw_line(start_pos, end_pos, ropeColor, lineWith)
	
	# 绘制节点
	for i in range(positions.size()):
		var pos = positions[i] * scale_factor + screen_center
		
		# 根据状态选择颜色和大小
		var node_color: Color
		var node_size: float
		
		if is_dragging and i == dragged_node_index:
			# 拖拽中的节点
			node_color = Color.CYAN
			node_size = nomal_node_size +3
		elif is_node_locked(i):
			# 锁定的节点
			node_color = Color.DARK_BLUE
			node_size = nomal_node_size + 2
		else:
			# 普通节点
			node_color = ropeColor
			node_size = nomal_node_size
		
		draw_node.draw_circle(pos, node_size, node_color)
		
		# 为锁定节点添加额外的视觉标识
		if is_node_locked(i):
			draw_node.draw_circle(pos, node_size + 3.0, Color.DEEP_PINK, false, 3.0)
		
		# 仅为关键节点添加标签
		if i == 0 or i == positions.size() - 1 or (debug_enabled and i < 3):
			var font = ThemeDB.fallback_font
			draw_node.draw_string(font, pos + Vector2(8, 0), str(i), HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Color.WHITE)

func remove_middle_node():
	# ✅ 修复：使用实际的节点数量
	var current_count = rope_simulator.get_current_node_count()
	if current_count > 3:
		var middle_index = current_count / 2
		rope_simulator.remove_node(middle_index)
		# 添加索引更新
		update_locked_nodes_after_removal(middle_index)
		positions_dirty = true



func toggle_debug():
	debug_enabled = !debug_enabled
	print("调试模式: ", "开启" if debug_enabled else "关闭")
	if debug_enabled:
		print_debug_info()

func reset_rope_system():
	rope_simulator.reset_rope()
	setup_rope_parameters()
	setup_constraints()
	positions_dirty = true

func print_debug_info():
	"""简化的调试信息输出"""
	var positions = get_cached_positions()
	if positions.is_empty():
		return
		
	print("节点数: %d, 首节点: %s, 末节点: %s" % [
		positions.size(), 
		str(positions[0]), 
		str(positions[-1])
	])

 # 添加右键点击处理函数
 # 修改 handle_right_click 函数，确保在拖拽过程中也能正确固定节点
func handle_right_click(mouse_pos: Vector2):

	"""处理右键点击事件 - 直接切换节点固定状态"""
	var clicked_node = get_clicked_node(mouse_pos)
	if clicked_node == -1:
		return

	# 如果正在拖拽且右击的是被拖拽的节点，特殊处理
	if is_dragging and clicked_node == dragged_node_index:
		# 停止拖拽并固定节点
		stop_drag()
	# 确保节点被正确固定
	if not locked_nodes.has(clicked_node):
		locked_nodes.append(clicked_node)
		rope_simulator.setNodeLocked(clicked_node, true)
	if debug_enabled:
					print("拖拽中右击固定节点: ", clicked_node)
	else:
		# 正常的右击固定/解锁切换
		toggle_node_lock(clicked_node)

	if debug_enabled:
		print("右键点击节点: ", clicked_node)

# 添加单击确认回调函数
func _on_single_click_confirmed():
	"""单击确认回调 - 切换节点固定状态"""
	if pending_click_node != -1:
		toggle_node_lock(pending_click_node)
		pending_click_node = -1

# 在文件末尾添加缺失的函数

# 设置约束函数
func setup_constraints():
	"""设置绳索约束，锁定第一个节点"""
	rope_simulator.setNodeLocked(0, true)  # 锁定第一个节点
	locked_nodes.clear()
	locked_nodes.append(0)
	if debug_enabled:
		print("约束设置完成 - 第一个节点已锁定")

# 获取缓存位置函数
func get_cached_positions() -> Array:
	"""获取缓存的节点位置"""
	if positions_dirty or cached_positions.is_empty():
		cached_positions.clear()
		# ✅ 修复：使用实际的节点数量
		var node_count = rope_simulator.get_current_node_count()
		for i in range(node_count):
			cached_positions.append(rope_simulator.get_node_position(i))
		positions_dirty = false
	return cached_positions

# 检查节点是否锁定
func is_node_locked(node_index: int) -> bool:
	"""检查节点是否被锁定"""
	return locked_nodes.has(node_index)

# 切换节点锁定状态
func toggle_node_lock(node_index: int):
	"""切换节点的固定状态"""
	if node_index < 0 or node_index >= rope_simulator.get_current_node_count():
		return
	
	# 获取当前锁定状态
	var is_currently_locked = is_node_locked(node_index)
	
	# 切换锁定状态
	rope_simulator.setNodeLocked(node_index, not is_currently_locked)
	
	# 更新锁定状态跟踪
	if is_currently_locked:
		locked_nodes.erase(node_index)
	else:
		if not locked_nodes.has(node_index):
			locked_nodes.append(node_index)
	
	if debug_enabled:
		var status = "解锁" if is_currently_locked else "锁定"
		print("节点 ", node_index, " 已", status)

# 更新检查节点锁定状态的函数
# # 取消注释并修复 is_node_locked 函数（大约在第85行）
# func is_node_locked(node_index: int) -> bool:
# 	"""检查节点是否被锁定"""
# 	return locked_nodes.has(node_index)

func update_drag(mouse_pos: Vector2):
	"""更新拖拽位置"""
	if not is_dragging or dragged_node_index == -1:
		return
	
	# 计算新的世界坐标位置
	var target_screen_pos = mouse_pos - drag_offset
	var target_world_pos = (target_screen_pos - screen_center) / scale_factor
	
	# 使用move_node而不是set_node_position，保持速度连续性
	var current_pos = rope_simulator.get_node_position(dragged_node_index)
	var displacement = target_world_pos - current_pos
	rope_simulator.move_node(dragged_node_index, displacement)
	positions_dirty = true
	
	if debug_enabled:
		print("拖拽节点 ", dragged_node_index, " 到位置: ", target_world_pos)

func get_clicked_node(mouse_pos: Vector2) -> int:
	"""获取鼠标点击位置最近的节点索引"""
	var positions = get_cached_positions()
	if positions.is_empty():
		return -1
	
	# 查找最近的节点
	var closest_index = -1
	var closest_distance = drag_threshold
	
	for i in range(positions.size()):
		var node_screen_pos = positions[i] * scale_factor + screen_center
		var distance = mouse_pos.distance_to(node_screen_pos)
		
		if distance < closest_distance:
			closest_distance = distance
			closest_index = i
	
	return closest_index

func cut_rope_from_node(node_index: int):
	"""从指定节点剪断到最后一个节点"""
	var current_count = rope_simulator.get_current_node_count()
	
	# 检查有效性
	if node_index < 0 or node_index >= current_count:
		print("节点索引无效")
		return
	
	# 不能剪断第一个节点（起始点）
	if node_index == 0:
		print("错误：不能剪断第一个节点")
		return
	
	# 如果点击的是最后一个节点，则移除它
	if node_index == current_count - 1:
		rope_simulator.remove_node(node_index)
		# 更新锁定节点索引
		update_locked_nodes_after_removal(node_index)
		positions_dirty = true
		print("移除了最后一个节点: ", node_index)
		return
	
	# 计算需要移除的节点数量（从点击节点到最后）
	var nodes_to_remove = current_count - node_index
	
	# 从后往前移除节点，避免索引变化问题
	for i in range(nodes_to_remove):
		var remove_index = current_count - 1 - i
		if remove_index >= node_index:
			rope_simulator.remove_node(remove_index)
	
	#更新锁定节点索引
	update_locked_nodes_after_batch_removal(node_index)
	
	positions_dirty = true
	print("从节点 ", node_index, " 剪断到末尾，移除了 ", nodes_to_remove, " 个节点")

# 新增：单个节点移除后的索引更新
func update_locked_nodes_after_removal(removed_index: int):
	"""更新单个节点移除后的锁定索引"""
	var updated_locked_nodes = []
	
	for locked_index in locked_nodes:
		if locked_index < removed_index:
			# 在移除节点之前的索引保持不变
			updated_locked_nodes.append(locked_index)
		elif locked_index > removed_index:
			# 在移除节点之后的索引需要减1
			updated_locked_nodes.append(locked_index - 1)
		# locked_index == removed_index 的节点被移除，不添加到新数组
	
	locked_nodes = updated_locked_nodes
	if debug_enabled:
		print("更新锁定节点索引: ", locked_nodes)

# 新增：批量节点移除后的索引更新
func update_locked_nodes_after_batch_removal(cut_index: int):
	"""更新批量节点移除后的锁定索引"""
	var updated_locked_nodes = []
	
	for locked_index in locked_nodes:
		if locked_index < cut_index:
			# 只保留在切断点之前的锁定节点
			updated_locked_nodes.append(locked_index)
		# 切断点及之后的所有锁定节点都被移除
	
	locked_nodes = updated_locked_nodes
	if debug_enabled:
		print("批量移除后更新锁定节点索引: ", locked_nodes)

# 修改 stop_drag 函数，添加可选参数控制是否解锁节点
func stop_drag(keep_locked: bool = false):
	"""停止拖拽"""
	if is_dragging:
		# 根据参数决定是否解锁节点
		if dragged_node_index != -1 and not keep_locked:
			# 检查节点是否应该保持锁定状态
			var should_stay_locked = locked_nodes.has(dragged_node_index)
			rope_simulator.setNodeLocked(dragged_node_index, should_stay_locked)
			# 如果是第一个节点，始终保持锁定
			if dragged_node_index == 0:
				rope_simulator.setNodeLocked(0, true)
		
		if debug_enabled:
			print("停止拖拽节点: ", dragged_node_index)
	
	is_dragging = false
	dragged_node_index = -1
	drag_offset = Vector2.ZERO

# 在 _ready() 函数之后添加
func _process(delta):
	"""每帧更新物理模拟和绘制"""
	# 更新物理
	rope_simulator.update_physics(delta)
	positions_dirty = true
	
	# 减少重绘频率，仅在需要时重绘
	if positions_dirty:
		draw_node.queue_redraw()
	
	# 调试信息
	if debug_enabled:
		frame_counter += 1
		if frame_counter % 120 == 0: 
			print_debug_info()
