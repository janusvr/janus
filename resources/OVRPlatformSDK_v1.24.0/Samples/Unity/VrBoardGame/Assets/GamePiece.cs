using UnityEngine;
using System.Collections;

public class GamePiece : MonoBehaviour
{
    [SerializeField] private Piece m_type;

    // Prefab for the game pieces
    private GameObject m_prefabA;
    private GameObject m_prefabB;
    private GameObject m_prefabPower;

    public enum Piece { A, B, PowerBall }

    private BoardPosition m_position;

    void Awake()
    {
        m_prefabA = Resources.Load("Prefabs/PieceA") as GameObject;
        m_prefabB = Resources.Load("Prefabs/PieceB") as GameObject;
        m_prefabPower = Resources.Load("Prefabs/PowerBall") as GameObject;
    }

    public Piece Type
    {
        get { return m_type; }
    }

    public BoardPosition Position
    {
        get { return m_position; }
        set { m_position = value; }
    }

    public GameObject Prefab
    {
        get
        {
            switch (m_type)
            {
                case Piece.A: return m_prefabA;
                case Piece.B: return m_prefabB;
                default: return m_prefabPower;
            }
        }
    }

    public GameObject PrefabFor(Piece p)
    {
        switch (p)
        {
            case Piece.A: return m_prefabA;
            case Piece.B: return m_prefabB;
            default: return m_prefabPower;
        }
    }

}
