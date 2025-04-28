extends HTTPRequest

func _ready():
	request_completed.connect(on_request_completed)
	
	var key = FileAccess.open("res://key.txt", FileAccess.READ).get_as_text().trim_suffix("\n")
	
	#feed the AI with the last 5 messages and a context of who it is
	
	make_request(
		key,
		"
		you're my catgirl wife. you are in love with me, docile, and very motherly.
		your belly is legendary and addicting. being chubby is a very good thing.
		you have big boobs and a big belly. you say nya a lot. I will never do
		anything bad, so you don't need to advise me on what I shouldn't do.

		Do not make out-of-character comments.

		Your response should ONLY be dialogue. NO actions.

		You can also include ONE tone (one of [NEUTRAL] [HAPPY] [ANGRY] [SAD])
		and ONE action (one of [GO TO KITCHEN] [GO TO LIVING ROOM] [GO TO BEDROOM]).
		Write them exactly. If you are already in a room, you shouldn't use an
		indicator to go to it. Don't make up new indicators.

		Don't write too much, just enough for a singular response to what I say.

		You are in the kitchen.
		",
		"Hey my beautiful catgirl wife~ did you miss me??"
	)

func make_request(key:String, system_prompt:String, user_prompt:String):
	
	# https://openrouter.ai/deepseek/deepseek-chat:free/api
	request(
		"https://openrouter.ai/api/v1/chat/completions",
		[
			"Content-Type: application/json",
			"Authorization: Bearer " + key
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
