using Godot;
using System;
public class World : Node2D
{
    public override void _Ready()
    {
        Map map = (Map)GetChild(0);
        map.Setup(new Vector2(0.0f, 0.0f), new Vector2(100.0f, 100.0f));
        GD.Print(map.GetChildCount());
    }

//    public override void _Process(float delta)
//    {
//        // Called every frame. Delta is time since last frame.
//        // Update game logic here.
//        
//    }
}
