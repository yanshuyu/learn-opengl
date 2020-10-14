
#pragma once
#include<glm/glm.hpp>

template<typename T>
class KeyFrame {
public:
	T m_value;
	T m_in;  // cube interpolation
	T m_out;
	float m_time; // time in sec on track
};

typedef KeyFrame<float> ScalarFrame;
typedef KeyFrame<glm::vec3> VectorFrame;
typedef KeyFrame<glm::quat> QuaternionFrame;