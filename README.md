# K2PostIt (UE5.4 to UE5.6)

This is a fairly small plugin which is intended to look down upon Unreal's very annoying "Comment" node. Don't get me wrong, the "Comment" node is great for wrapping/labelling blocks of blueprint graph, but it's hardly a comment node!

This plugin is currently for the Blueprint Graph only. More graph types may be supported later.

## Demo Video

https://github.com/user-attachments/assets/5fc84412-b88a-4fef-95f2-b23d7bb4b7c4

## Usage/Features
- Press Alt+C to place a new comment node.
- Markdown-style support (supports basic markdown; advanced markdown capabilities may be added later)
- Realtime rendered markdown preview pane.
- Preset quick-selectable colors, editable in project settings.

## Notes/Known Issues
- Currently tested for UE 5.4+ only. It may be easy to make this plugin work on older UE5 versions (the main limit for older versions is due to TInstancedStruct usage).
- Not all markdown is supported. The preview video above shows currently available formatting.
- The `inline code` markdown style cannot auto-wrap yet.
  
### Normal display mode
<img width="768" height="636" alt="image" src="https://github.com/user-attachments/assets/39c3a29e-85cc-45d2-b216-a3afbda98ee3" />

### Editing mode
<img width="1405" height="638" alt="image" src="https://github.com/user-attachments/assets/35d5357f-f48a-4b97-9efc-3882a39f57b6" />

### Project settings
<img width="913" height="531" alt="image" src="https://github.com/user-attachments/assets/88fa0702-bd12-46ea-8d10-07d55f9da884" />
