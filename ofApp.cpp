#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(60);
	ofSetWindowTitle("openFrameworks");

	ofBackground(239);
	ofEnableDepthTest();
	ofDisableArbTex();

	this->cap_size = cv::Size(640, 360);

	this->cap.open(0);
	this->cap.set(cv::CAP_PROP_FPS, 30);
	this->cap.set(cv::CAP_PROP_FRAME_WIDTH, this->cap_size.width);
	this->cap.set(cv::CAP_PROP_FRAME_HEIGHT, this->cap_size.height);

	this->rect_size = 20;
	for (int x = 0; x < this->cap_size.width; x += this->rect_size) {

		for (int y = 0; y < this->cap_size.height; y += this->rect_size) {

			auto image = make_unique<ofImage>();
			image->allocate(this->rect_size, this->rect_size, OF_IMAGE_COLOR);
			cv::Mat frame = cv::Mat(cv::Size(image->getWidth(), image->getHeight()), CV_MAKETYPE(CV_8UC3, image->getPixels().getNumChannels()), image->getPixels().getData(), 0);
			cv::Rect rect = cv::Rect(x, y, this->rect_size, this->rect_size);

			this->rect_images.push_back(move(image));
			this->cv_rects.push_back(rect);
			this->rect_frames.push_back(frame);
		}
	}

	this->face_cascade.load("D:\\opencv_4.1.2\\build\\install\\etc\\haarcascades\\haarcascade_frontalface_alt2.xml");
}

//--------------------------------------------------------------
void ofApp::update() {

	cv::Mat frame, blend;
	if (!this->cap.isOpened()) { return; }

	this->cap >> this->frame;
	if (this->frame.empty()) { return; }

	cv::cvtColor(this->frame, this->frame, cv::COLOR_RGB2BGR);
	cv::flip(this->frame, this->frame, 1);

	if (ofGetFrameNum() == 0) { this->frame.copyTo(this->save_frame); }

	cv::Mat gray_frame, small_frame;
	cv::cvtColor(this->frame, gray_frame, cv::COLOR_RGB2GRAY);
	cv::resize(gray_frame, small_frame, cv::Size(this->cap_size.width * 0.5, this->cap_size.height * 0.5));

	vector<cv::Rect> faces;
	this->face_cascade.detectMultiScale(small_frame, faces);

	int face_count = 0;
	this->face_location = glm::vec2();
	this->face_size = glm::vec2();
	float max_area = 0.f;
	for (cv::Rect r : faces) {

		auto tmp_face_location = glm::vec2(r.x * 2.f, r.y * 2.f);
		auto tmp_face_size = glm::vec2(r.width * 2, r.height * 2);
		auto area = r.width * r.height;
		if (area > max_area) {

			this->face_location = tmp_face_location;
			this->face_size = tmp_face_size;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	this->cam.begin();
	ofTranslate(this->cap_size.width * -0.5, this->cap_size.height * -0.5, 0);

	for (int i = 0; i < this->rect_frames.size(); i++) {

		cv::Mat tmp_box_image(this->save_frame, this->cv_rects[i]);
		tmp_box_image.copyTo(this->rect_frames[i]);

		if (this->cv_rects[i].x > this->face_location.x - 20 - 40 && this->cv_rects[i].x < this->face_location.x - 20 + this->face_size.x + 0
			&& this->cv_rects[i].y > this->face_location.y - 20 - 40 && this->cv_rects[i].y < this->face_location.y - 20 + this->face_size.y + 0) {
		
			auto noise_value = ofNoise(this->cv_rects[i].x, this->cv_rects[i].y, ofGetFrameNum() * 0.2);

			if (noise_value < 0.55) {

				cv::Mat tmp_box_image(this->save_frame, this->cv_rects[i]);
				tmp_box_image.copyTo(this->rect_frames[i]);
			}
			else {

				cv::Mat tmp_box_image(this->frame, this->cv_rects[i]);
				tmp_box_image.copyTo(this->rect_frames[i]);
			}
		}
		else {

			cv::Mat tmp_box_image(this->frame, this->cv_rects[i]);
			tmp_box_image.copyTo(this->rect_frames[i]);
		}

		this->rect_images[i]->update();
		this->rect_images[i]->getTexture().bind();
		ofSetColor(255);
		ofFill();
		ofDrawBox(glm::vec3(this->cv_rects[i].x + 10, this->cap_size.height - this->cv_rects[i].y - 10, 0), this->rect_size, this->rect_size, 5);
		this->rect_images[i]->unbind();

		ofSetColor(39);
		ofNoFill();
		ofDrawBox(glm::vec3(this->cv_rects[i].x + 10, this->cap_size.height - this->cv_rects[i].y - 10, 0), this->rect_size, this->rect_size, 5);
	}

	this->cam.end();
}

//--------------------------------------------------------------
int main() {

	ofSetupOpenGL(720, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}