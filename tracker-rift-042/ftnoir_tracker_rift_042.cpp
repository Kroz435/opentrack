/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_rift_042.h"
#include "opentrack/plugin-api.hpp"
#include "OVR_CAPI.h"
#include "Kernel/OVR_Math.h"
#include <cstdio>
#include <cstring>

using namespace OVR;

Rift_Tracker::Rift_Tracker() : old_yaw(0), hmd(nullptr)
{
}

Rift_Tracker::~Rift_Tracker()
{
    ovrHmd_Destroy(hmd);
    ovr_Shutdown();
}

void Rift_Tracker::start_tracker(QFrame*)
{
    ovr_Initialize();
    hmd = ovrHmd_Create(0);
    if (hmd)
    {
        ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, ovrTrackingCap_Orientation);
    }
    else
    {
        // XXX need change ITracker et al api to allow for failure reporting
        // this qmessagebox doesn't give any relevant details either -sh 20141012
        QMessageBox::warning(0,"Error", "Unable to start Rift tracker",QMessageBox::Ok,QMessageBox::NoButton);
    }
}


void Rift_Tracker::data(double *data)
{
    if (hmd)
    {
        ovrHSWDisplayState hsw;
        std::memset(&hsw, 0, sizeof(hsw));
        ovrHmd_GetHSWDisplayState(hmd, &hsw);
        if (hsw.Displayed)
            ovrHmd_DismissHSWDisplay(hmd);
        ovrTrackingState ss = ovrHmd_GetTrackingState(hmd, 0);
        if (ss.StatusFlags & ovrStatus_OrientationTracked)
        {
            auto pose = ss.HeadPose.ThePose;
            Quatf quat = pose.Orientation;
            float yaw, pitch, roll;
            quat.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);
            if (s.useYawSpring)
            {
                yaw = old_yaw*s.persistence + (yaw-old_yaw);
                if(yaw > s.deadzone)
                    yaw -= s.constant_drift;
                if(yaw < -s.deadzone)
                    yaw += s.constant_drift;
                old_yaw=yaw;
            }
            constexpr double d2r = 57.295781;
            data[Yaw] = yaw * -d2r;
            data[Pitch] = pitch * d2r;
            data[Roll] = roll * d2r;
            data[TX] = pose.Position.x * -1e2;
            data[TY] = pose.Position.y *  1e2;
            data[TZ] = pose.Position.z *  1e2;
        }
    }
}

OPENTRACK_DECLARE_TRACKER(Rift_Tracker, TrackerControls, FTNoIR_TrackerDll)
