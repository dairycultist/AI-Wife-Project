extends HTTPRequest

# ../Library/Application Support/Godot/app_userdata/AI Wife Project/key.txt 
var KEY = FileAccess.open("user://key.txt", FileAccess.READ).get_as_text().trim_suffix("\n")

var prompting = false

func _input(event) -> void:
		
	if event.is_action_pressed("enter"):
		
		prompting = not prompting
		
		if prompting:
			$LineEdit.visible = true
			$LineEdit.edit()
		else:
			
			var regex = RegEx.new()
			regex.compile("^ *(?<prompt>\\S|\\S\\S|\\S.*\\S) *$")
			var result = regex.search($LineEdit.text)
			if result:
				print(result.get_string("prompt"))
			
			$LineEdit.visible = false
			$LineEdit.unedit()
			$LineEdit.text = ""
		
		# feed the AI with the last 5 messages and a context of who it is
		#make_request(
			#"
			#you're my catgirl wife. you are in love with me, docile, and very motherly.
			#you have big boobs and a big belly. you say nya a lot. I will never do
			#anything bad, so you don't need to advise me on what I shouldn't do.
#
			#Do not make out-of-character comments.
#
			#Your response should ONLY be dialogue, do not use actions/italics.
#
			#You can also include one of [NEUTRAL] [HAPPY] [ANGRY] [SAD].
			#Write them exactly. Don't make up new emotions.
#
			#Don't write too much, just enough for a singular response to what I say.
			#",
			#"Hey my beautiful catgirl wife~ did you miss me??"
		#)


func _ready():
	request_completed.connect(on_request_completed)

func make_request(system_prompt:String, user_prompt:String):
	
	# https://openrouter.ai/deepseek/deepseek-chat:free/api
	request(
		"https://openrouter.ai/api/v1/chat/completions",
		[
			"Content-Type: application/json",
			"Authorization: Bearer " + KEY
		],
		HTTPClient.METHOD_POST,
		'
			{
				"model": "deepseek/deepseek-chat:free",
				"messages": [
					{ "role": "system", "content": "' + system_prompt + '" },
					{ "role": "user", "content": "' + user_prompt + '" }
				]
			}
		'
	)

func on_request_completed(result, response_code, headers, body):
	
	if response_code == 200:
		
		var json = JSON.parse_string(body.get_string_from_utf8())
		print(json["choices"][0]["message"]["content"])
		
	else:
		
		print(str(result) + "\n\n")
		print(str(response_code) + "\n\n")
		print(str(headers) + "\n\n")
		print(body.get_string_from_utf8())
