using Godot;
using System;
public class Map : Node2D
{
    class Edge {
        public Vector2[] points;

        // Delunay Triangulation. d0.centre -> d1.centre
        public Tile d0;
        public Tile d1;

        public Vector2 v0() {
            return points[0];
        }

        public Vector2 v1() {
            return points[points.Length - 1];
        }
    }

    class TileEdge {
        public Edge sharedEdge;
        public Vector2[] points;
        public Tile neighbour;

        public Vector2 v0() {
            return points[0];
        }

        public Vector2 v1() {
            return points[points.Length - 1];
        }
    }

    class Tile {
        public Vector2 centre;
        public TileEdge[] edges;
        public bool usable;

        public const int EDGE_DETAIL = 0; // Total number of points in an edge = 1 << EDGE_DETAIL + 2

        public double edgeAngle(Edge e) {
            Vector2 centre_offset = (e.v0() + e.v1()) * 0.5f - centre;
            return Mathf.Atan2(centre_offset.x, centre_offset.y);
        }

        public double vertexAngle(Vector2 v) {
            return Mathf.Atan2(v.x - centre.x, v.y - centre.y);
        }
    }

    public void Setup(Vector2 min, Vector2 max) {
        Clear();
        
        var polygon = new Polygon2D();
        polygon.Polygon = new Vector2[]{
            new Vector2(0.0f, 0.0f),
            new Vector2(0.0f, 100.0f),
            new Vector2(100.0f, 100.0f),
            new Vector2(100.0f, 0.0f)
        };
        polygon.Color = new Color(0.5f, 0.5f, 0.8f);
        this.AddChild(polygon);

        var polygon2 = new Polygon2D();
        polygon2.Polygon = new Vector2[]{
            new Vector2(100.0f, 0.0f),
            new Vector2(100.0f, 100.0f),
            new Vector2(200.0f, 100.0f),
            new Vector2(200.0f, 0.0f)
        };
        polygon2.Color = new Color(0.1f, 0.5f, 0.8f);
        this.AddChild(polygon2);
    }

    public void Clear() {
        foreach (Node child in GetChildren()) {
            child.QueueFree();
        }
    }

    public override void _Ready()
    {
    }

//    public override void _Process(float delta)
//    {
//        // Called every frame. Delta is time since last frame.
//        // Update game logic here.
//        
//    }
}
