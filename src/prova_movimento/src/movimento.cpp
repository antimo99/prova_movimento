#include <ros/ros.h>
#include <control_msgs/FollowJointTrajectoryAction.h>
#include <actionlib/client/simple_action_client.h>
#include <sensor_msgs/JointState.h>

//vairabile globale per la lettura della configurazione iniziale
sensor_msgs::JointState q0;
bool message_received = false;

void messageCallback(const sensor_msgs::JointState::ConstPtr& msg)
{

    q0 = *msg;  // Salva il messaggio ricevuto
    message_received = true;  // Imposta la variabile di stato
    if(message_received)
    {
        //ROS_INFO("Configurazione iniziale letta: %s",q0.position.toString().c_str());
        //q0.position.toString().c_str()
    }
}



int main (int argc, char **argv)
{
  ros::init(argc, argv, "nodo_client");

  std::vector<std::string> joint_names_ = {"panda_joint1",
                                           "panda_joint2",
                                           "panda_joint3",
                                           "panda_joint4",
                                           "panda_joint5",
                                           "panda_joint6",
                                           "panda_joint7"};

  // create the action client
  // true causes the client to spin its own thread
  actionlib::SimpleActionClient<control_msgs::FollowJointTrajectoryAction> ac("/position_joint_trajectory_controller/follow_joint_trajectory", true);


  /**
  * NodeHandle is the main access point to communications with the ROS system.
  * The first NodeHandle constructed will fully initialize this node, and the last
  * NodeHandle destructed will close down the node.
  */
  ros::NodeHandle n;
  ros::Subscriber subscriber = n.subscribe("joint_states", 1000, messageCallback);
  
  // Attendi il singolo messaggio e salvalo nella variabile globale
  while (ros::ok() && !message_received)
  {
     ros::spinOnce();  // Esegui le callback per gestire i messaggi
  }

  // wait for the action server to start
  ROS_INFO("Waiting for action server to start.");
  ac.waitForServer(); //will wait for infinite time

  ROS_INFO("Action server started, sending goal.");
  // send a goal to the action
  control_msgs::FollowJointTrajectoryActionGoal goal;
  goal.goal.trajectory.joint_names=joint_names_;

  trajectory_msgs::JointTrajectory vec;
  vec.points.resize(2);
  vec.points[0].positions=q0.position;
  vec.points[1].positions={q0.position[0],q0.position[1],q0.position[2],q0.position[3],q0.position[4],q0.position[5],q0.position[6]+M_PI/2};
  goal.goal.trajectory.points=vec.points;
  
  //invia la richiesta all'action_server
  ac.sendGoal(goal.goal);

  //wait for the action to return
  bool finished_before_timeout = ac.waitForResult(ros::Duration(30.0));

  if (finished_before_timeout)
  {
    actionlib::SimpleClientGoalState state = ac.getState();
    ROS_INFO("Action finished: %s",state.toString().c_str());
  }
  else
    ROS_INFO("Action did not finish before the time out.");

  //exit
  return 0;
}


//callback per la lettura dal topic
