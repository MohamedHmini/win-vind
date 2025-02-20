---
layout: page
title: Usage - win-vind
nav: Usage
icon: map-signs
order: 0
disable_anchors: true
---
This software only supports Windows 10 or Windows 11 on real machines. Therefore, it may not work on older Windows or virtual environments such as Wine or Virtual Box. If you have any problems or requests, please post them to [GitHub Issues](https://github.com/pit-ray/win-vind/issues).  


## Installation

- Download the latest version of the type that suits your preference from the [Downloads page]({{ site.url }}/downloads) and install it.

- Install and run win-vind.exe.

- If you can see the icon in the system tray, it is working properly.  
   <p align="center">
   <img src="{{ site.url }}/imgs/taskbar.jpg" width=500 >  
   <p align="center">Like this</p>
   </p>


## Note 
- `:exit` is the recommended termination.
- `<F8> + <F9>` is safe forced termination.
- win-vind could **not** operate some windows given high-rank authorization than itself. For example, if you start **Task Manager** having the highest authorization and select its window, you cannot move, click or scroll the mouse cursor by win-vind. If you want to operate all windows, I recommend giving win-vind the administrator authorization. (Please use **Task Scheduler**).


## Quick Tutorial

### 1. Mode Transition

The basic concept is the same as Vim, but there are two **Normal Mode** and two **Visual Mode**, and **Resident Mode**.   

<p align="center">
<img src="{{ site.url }}/imgs/mode_overview.png" width=600>  
<p align="center">Default mode layer overview</p>
</p>

There are two groups: GUI mode and Editor mode.
The former allows us to control windows and mouse cursor, etc. The latter allows us to emulate Vim in input forms of web pages, Microsoft Office Word, etc.  

<p align="center">
<img src="{{ site.url }}/imgs/GUIandEditor.jpg" width=700>
<p align="center">Concepts of GUI Mode and Editor Mode</p>
</p>

Resident Mode is an evacuation mode to prevent bindings from being collisions with shortcut keys while gaming on Steam or using Vim. For example, if you have added `<Esc>` into the keymap of Insert Mode for faithful Vim emulation, you can use Resident Mode to prevent win-vind from being called by Vim's `<Esc>`.  


**Insert Mode** and **Resident Mode** pass all key messages to Windows, while **GUI Normal Mode**, **GUI Visual Mode**, **Edi Normal Mode**, **Edi Visual Mode**, and **Command Mode** block them.  

<p align="center">
<img src="{{ site.url }}/imgs/mode_overview_3D.png" width=500 >  
<p align="center">Visual Concepts of Mode</p>
</p>

After the boot, win-vind will be in **Insert Mode**. Let's make transitions of mode!  


### 2. GUI Operation and Window Operation  

1. Switch to **GUI Normal Mode** with `<Esc-Left>`.
1. Please inputs `:!mspaint` to launch Microsoft Paint.
1. You can call **EasyClick** with `FF`.
   <p align="center">
   <img src="{{ site.url }}/imgs/EasyClickDemo.gif">
   <p align="center">EasyClick Demo</p>
   </p>
1. Let's select windows with `<C-w>h` or `<C-w>l`.
1. Please select Microsoft Paint and close it with `:close`.



### 3. Customize win-vind  

win-vind uses **Run Commands** style configuration method. If you've ever written a `.vimrc`, it's easy to make it your win-vind. 

Generally, there are three levels of key mapping: **key2key**, **keyset2keyset**, and **cmd2cmd**. key2key assigns one key to another. keyset2keyset assigns one key combination to another, such as `<c-s>` to `<m-h>`. cmd2cmd is a mapping scheme that generates another command in response to a sequential command input, such as `qq` to `<c-w>e`.  

The keyset syntax uses the same expression as in Vim, where keys are connected by `-` between `<` and `>`. However, there is no limit to the number of combinations, and you can connect as many as you like. (e.g. `<Esc-b-c-a-d>`).  

The following commands are supported. By the way, `{` and `}` themselves are not part of the syntax.  

|**Syntax**|**Effects**|
|:---|:---|
|`set {option}`|Set the value of the option to **true**.|
|`set no{option}`|Set the value of the option to **false**.|
|`set {option} = {val}`|Set a value of the option. The value can be a string or a number that allows floating points. The string does not need quotation marks, and any character after the non-white character will be handled as the value. White spaces at both ends of the equals sign are ignored.|
|`{mode}map {in-key} {out-key}`|It performs **cmd2cmd** mapping with low-level. The defined low-level map is actually propagated to Windows as keystroke. By the way, only the key2keyset format can synchronize the key state without delay.|
|`{mode}noremap {in-cmd} {func-id}`|It defines the map to call the function.|
|`{mode}noremap {in-keyset} {out-keyset}`|It performs **cmd2cmd** mapping in win-vind scope. However, since the `{func-id}` definition has higher priority than its syntax, it may result in exactly one level of recursive mapping.|
|`{mode}unmap {in-cmd}`|Remove the map corresponding to the `{in-cmd}`.|
|`{mode}mapclear`|Delete all maps.|
|`command {in-cmd} {func-id}`|It defines the command to call the function.|
|`delcommand {in-cmd}`|Remove the command corresponding to the `{in-cmd}`.|
|`comclear`|delete all commands.|
|`source`|Load another .vindrc. Either fill in the path to the .vindrc or use the syntax `user/repo` to load the .vindrc in the root directory of the GitHub repository.|

`{mode}` is the [Mode Prefix]({{ site.url }}/cheat_sheet/keywords/#mode-prefix). And only **UTF-8** format is supported for `.vindrc`.  


Let's do the last tutorial!  

1. Go to **Insert Mode**.
1. This time, we will try **Instant GUI Normal Mode** with `<F8>`. It allows us to temporarily switch to the **GUI Normal Mode**.  

   <p align="center">
   <img src="{{ site.url }}/imgs/instant_gui_normal_mode.jpg" width=450 >  
   <p align="center">Instant GUI Normal Demo</p>
   </p>
1. Open your `.vindrc` with `:e`.
   <p align="center">
   <img src="{{ site.url }}/imgs/edit_vindrc_demo.jpg" width=450 >  
   <p align="center">Edit .vindrc Demo</p>
   </p>
1. Write following commands into `.vindrc`.
   ```vim
   set cmd_fontname = Times New Roman
   imap <Capslock> <ctrl>
   inoremap <Alt> easy_click_left
   imap <ctrl-a> Vim is the best editor.
   gnnoremap <ctrl-1> :!notepad<cr>
   ```
1. If you done, try reloading `.vindrc` with `:source` of win-vind. (No arguments are needed.)
   <p align="center">
   <img src="{{ site.url }}/imgs/source_demo.jpg" width=450 >  
   <p align="center">Reload Demo</p>
   </p>
1. In **Insert Mode**, you can use `<Capslock>` instead of `<Ctrl>` and call EasyClick with a single `<Alt>`. And, you can insert a fixed form text with pressing Ctrl and A at the same time. **In GUI Normal Mode**, pressing Ctrl and 1 at the same time will open Notepad.

<br>
<br>
<br>
<br>
