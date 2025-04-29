extends Node3D

@export var leg1:Node3D
@export var leg2:Node3D
@export var leg1_probe:Node3D
@export var leg2_probe:Node3D

var leg1_target:Vector3
var leg2_target:Vector3

var step_cooldown := 0.0

func _ready() -> void:
	
	leg1_target = leg1.global_position + Vector3(0, -1, 0)
	leg2_target = leg2.global_position + Vector3(0, -1, 0)

func leg_raycast(probe:Node3D) -> Vector3:
	
	var space_state := get_world_3d().direct_space_state
	var query := PhysicsRayQueryParameters3D.create(probe.global_position, probe.global_position + Vector3(0, -100, 0))
	var result := space_state.intersect_ray(query)
	
	return result.position

func _process(delta:float) -> void:
	
	self.global_position = Vector3(0, 0, Time.get_ticks_msec() / 1000.0)
	
	if step_cooldown <= 0.0:
		
		var leg1_new_target := leg_raycast(leg1_probe)
		var leg2_new_target := leg_raycast(leg2_probe)
		
		var leg1_dist := leg1_new_target.distance_to(leg1_target)
		var leg2_dist := leg2_new_target.distance_to(leg2_target)
		
		if max(leg1_dist, leg2_dist) > 1.5:
			
			if leg1_dist > leg2_dist:
				leg1_target = leg1_new_target
			else:
				leg2_target = leg2_new_target
	
		step_cooldown = 1.0
	
	else:
		step_cooldown -= delta
	
	leg1.basis = lerp(leg1.basis, Basis.looking_at(leg1.global_position.direction_to(leg1_target), Vector3(0, 0, 1)), 10 * delta)
	leg2.basis = lerp(leg2.basis, Basis.looking_at(leg2.global_position.direction_to(leg2_target), Vector3(0, 0, 1)), 10 * delta)
