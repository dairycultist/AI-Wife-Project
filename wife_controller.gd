extends Node3D

@export var body:Node3D
@export var leg1:Node3D
@export var leg2:Node3D

var leg1_target:Vector3
var leg2_target:Vector3

func _ready() -> void:
	
	leg1_target = leg1.global_position + Vector3(0, -1, 0)
	leg2_target = leg2.global_position + Vector3(0, -1, 0)

func _process(delta: float) -> void:
	
	self.global_position = Vector3(Time.get_ticks_msec() / 1000.0, 0, 0)
	
	if (leg1.global_position.distance_to(leg1_target) > 1.5):
		leg1_target += Vector3(2, 0, 0)
	
	leg1.basis = lerp(leg1.basis, Basis.looking_at(leg1.global_position.direction_to(leg1_target), Vector3(0, 0, 1)), 5 * delta)
	leg2.basis = lerp(leg2.basis, Basis.looking_at(leg2.global_position.direction_to(leg2_target)), 5 * delta)
