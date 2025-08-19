# K2PostIt

This is a fairly small plugin which is intended to look down upon Unreal's very annoying "Comment" node. Don't get me wrong, the "Comment" node is great for wrapping/labelling blocks of blueprint graph, but it's hardly a comment node!

This plugin is currently for the Blueprint Graph only. More graph types may be supported later.

## Demo Video

This two minute video shows the node in action, and also includes some audio from Zombocom halfway through to keep you entertained:

https://i.gyazo.com/9f07ab41f66fcd87b5077657dd08a329.mp4

## Features
- Hold Shift+C and click to place a new comment node
- Preset quick-selectable colors (editable in project settings)
- Realtime markdown preview pane
- Partial markdown-style support (some complex edge cases will likely not work correctly, this system may improve over time)

  - *TODO 1: When placing a comment, if you don't let go of Shift+C right away, you'll get a bunch of CCCC's in the title.*
  - *TODO 2: This solution will modify your Saved\Config\EditorPerProjectUserSettings.ini file by adding a hard-coded entry to [BlueprintSpawnNodes]. I would like to improve this at some point (probably by making Shift+C place a node without clicking the same way that C places a normal comment node).*
  
### Normal display mode
<img width="768" height="636" alt="image" src="https://github.com/user-attachments/assets/39c3a29e-85cc-45d2-b216-a3afbda98ee3" />

### Editing mode
<img width="1405" height="638" alt="image" src="https://github.com/user-attachments/assets/35d5357f-f48a-4b97-9efc-3882a39f57b6" />

### Project settings
<img width="913" height="531" alt="image" src="https://github.com/user-attachments/assets/88fa0702-bd12-46ea-8d10-07d55f9da884" />

