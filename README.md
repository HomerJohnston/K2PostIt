# K2PostIt (UE5.4 to UE5.6)

This is a fairly small plugin which is intended to look down upon Unreal's very annoying "Comment" node. Don't get me wrong, the "Comment" node is great for wrapping/labelling blocks of blueprint graph, but it's hardly a comment node!

This plugin is currently for the Blueprint Graph only. More graph types may be supported later.

## Demo Video

This video shows the node in action (it appears to freeze near the end as the amount of text grows - this is just caused by reparsing all the regex on every keystroke; the node does not run the parser async (yet!)):

https://github.com/user-attachments/assets/10877b42-adfb-436b-8b38-955170b587fc

## Features
- Hold Shift+C and click to place a new comment node
- Preset quick-selectable colors (editable in project settings)
- Realtime markdown preview pane
- Partial markdown-style support (some complex edge cases will likely not work correctly, this system may improve over time)

## Notes/Known Nuisances
- When placing a comment, if you don't let go of Shift+C right away, you'll get a bunch of CCCC's in the title.
- This plugin will modify your Saved\Config\EditorPerProjectUserSettings.ini file by adding a hard-coded entry to [BlueprintSpawnNodes]. I would like to improve this at some point (probably by abandoning this system and making Shift+C place a node directly).*
- Not all markdown is supported.
- Longer notes can bog down in performance, especially with URLs. The parser should be made to run async.
  
### Normal display mode
<img width="768" height="636" alt="image" src="https://github.com/user-attachments/assets/39c3a29e-85cc-45d2-b216-a3afbda98ee3" />

### Editing mode
<img width="1405" height="638" alt="image" src="https://github.com/user-attachments/assets/35d5357f-f48a-4b97-9efc-3882a39f57b6" />

### Project settings
<img width="913" height="531" alt="image" src="https://github.com/user-attachments/assets/88fa0702-bd12-46ea-8d10-07d55f9da884" />

