using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net.Sockets;
using System.Text;
using UnityEngine;
using UnityEngine.UI;


public class PlayerAction : MonoBehaviour
{

    Animator anim;//Animation control 하기 위해서
    Rigidbody2D rigid;
    Vector3 dirVec;
    GameObject scanObject;

    public GameObject Player;//unity에서 직접 연결해줌
    public GameObject uiPost;
    public GameObject uiBoard;

    public float Speed;
    float x;
    float y;
    bool isHorizonMove;
    //public GameManager manager;
    private ClientManager clientManagerInstance;

    private void Awake()
    {
        rigid = GetComponent<Rigidbody2D>();
        anim = GetComponent<Animator>();
        
    }

    private void Start()
    {
        // ClientManager 스크립트의 인스턴스 생성
        GameObject clientManagerObject = GameObject.Find("ClientManager"); // ClientManager 게임 오브젝트를 찾음
        if (clientManagerObject != null)
        {
            clientManagerInstance = clientManagerObject.GetComponent<ClientManager>(); // ClientManager 스크립트의 인스턴스를 가져옴
        }
        else
        {
            Debug.LogError("Failed to find ClientManager object.");
        }
    }


    // Start is called before the first frame update
    void Update()
    {
        //move Value
        x = Input.GetAxisRaw("Horizontal");
        y = Input.GetAxisRaw("Vertical");

        //Check Button Down & Up
        bool xDown = Input.GetButtonDown("Horizontal");
        bool yDown = Input.GetButtonDown("Vertical");
        bool xUp = Input.GetButtonUp("Horizontal");
        bool yUp = Input.GetButtonUp("Vertical");

        //Check Horizontal Move
        if(xDown)
            isHorizonMove = true; 
        else if(yDown)
            isHorizonMove = false;
        else if(xUp || yUp)
        {
            isHorizonMove = x != 0;//위 아래 동시에 눌렀을 수평,수직을 동시에 눌렀을 때
        }

        //Animation, if문이 없다면 계속 다시 실행시켜야함-> 애니메이션 초기 프레임만 계속 실행된다.
        if (anim.GetInteger("xAxisRaw") != x)//해당 값이 X와 같다면(변화가 없다 면)
        {
            anim.SetBool("isChange", true);
            anim.SetInteger("xAxisRaw", (int)x);
        }
        else if(anim.GetInteger("yAxisRaw") != y)
        {
            anim.SetBool("isChange", true);
            anim.SetInteger("yAxisRaw", (int)y);
        }
        else
        {
            anim.SetBool("isChange", false);
        }

        //Direction, 우리가 바로보고 있는 방향
        if(yDown && y == 1)
            dirVec = Vector3.up;
        else if (yDown && y == -1)
            dirVec = Vector3.down;
        else if(xDown && x == -1)
            dirVec = Vector3.left;
        else if(xDown && x == 1)
            dirVec = Vector3.right;


        //Scanf Object
        if (Input.GetButtonDown("Jump") && scanObject != null)
        {
            if ( scanObject.name == "Table")
            {
                uiPost.SetActive(true);
                Rigidbody2D playerRigid = Player.GetComponent<Rigidbody2D>();
                playerRigid.constraints = RigidbodyConstraints2D.FreezeAll;
            }
            else if( scanObject.name  == "Board")
            {
                uiBoard.SetActive(true);
                Rigidbody2D playerRigid = Player.GetComponent<Rigidbody2D>();
                playerRigid.constraints = RigidbodyConstraints2D.FreezeAll;
                clientManagerInstance.open_post();
            }

        }

    }



    // Update is called once per frame
    void FixedUpdate()//물리 입력이 있을 때만 존재
    {
        Vector2 moveVec = isHorizonMove ? new Vector2(x, 0) : new Vector2(0, y);
        rigid.velocity = moveVec * 1.2f * Speed;

        // Ray, 스캔 할 때 사용,DrawRay먼저 해보면 RayCast 구현 더 쉽다.
        Debug.DrawRay(rigid.position, dirVec * 0.7f, new Color(0, 1, 0));
        // LayerMask.GetMask("Object") Object만 Scan 가능하다.
        RaycastHit2D rayHit = Physics2D.Raycast(rigid.position, dirVec, 0.7f, LayerMask.GetMask("Object"));

        if (rayHit.collider != null)
        {
            scanObject = rayHit.collider.gameObject;
        }
        else
            scanObject = null;

        
    }

    public void melt()// X 버튼 클릭시 플레이어 다시 움직이게 melt
    {
        Rigidbody2D playerRigid = Player.GetComponent<Rigidbody2D>();
        playerRigid.constraints = RigidbodyConstraints2D.None;
        playerRigid.constraints = RigidbodyConstraints2D.FreezeRotation;

    }
}
