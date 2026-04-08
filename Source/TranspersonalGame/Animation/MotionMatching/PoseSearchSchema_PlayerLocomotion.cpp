#include "PoseSearchSchema_PlayerLocomotion.h"
#include "PoseSearch/PoseSearchFeatureChannel_Trajectory.h"
#include "PoseSearch/PoseSearchFeatureChannel_Pose.h"
#include "PoseSearch/PoseSearchFeatureChannel_Velocity.h"
#include "PoseSearch/PoseSearchFeatureChannel_Position.h"

UPoseSearchSchema_PlayerLocomotion::UPoseSearchSchema_PlayerLocomotion()
{
    // Base configuration for survival gameplay
    SampleRate = 30.0f; // Higher precision for fear-based movements
    DataPreprocessor = EPoseSearchDataPreprocessor::Normalize;
    
    SetupLocomotionChannels();
    SetupFearChannels();
    SetupTerrainChannels();
}

void UPoseSearchSchema_PlayerLocomotion::PostLoad()
{
    Super::PostLoad();
    
    // Ensure fear and terrain systems are properly configured
    if (FearInfluenceWeight < 0.1f)
    {
        FearInfluenceWeight = 0.3f;
    }
    
    if (TerrainAdaptationWeight < 0.1f)
    {
        TerrainAdaptationWeight = 0.4f;
    }
}

void UPoseSearchSchema_PlayerLocomotion::PostInitProperties()
{
    Super::PostInitProperties();
    
    // Configure for paleontologist character movement
    // Emphasize cautious, observational movement patterns
}

void UPoseSearchSchema_PlayerLocomotion::SetupLocomotionChannels()
{
    // Trajectory Channel - Primary movement prediction
    UPoseSearchFeatureChannel_Trajectory* TrajectoryChannel = NewObject<UPoseSearchFeatureChannel_Trajectory>(this);
    TrajectoryChannel->Weight = 1.0f;
    
    // Add trajectory samples for cautious movement prediction
    FPoseSearchTrajectorySample ForwardSample;
    ForwardSample.Offset = 0.5f; // Look ahead for danger
    ForwardSample.Flags = EPoseSearchTrajectoryFlags::Position | EPoseSearchTrajectoryFlags::VelocityDirection;
    ForwardSample.Weight = 1.0f;
    TrajectoryChannel->Samples.Add(ForwardSample);
    
    FPoseSearchTrajectorySample NearFutureSample;
    NearFutureSample.Offset = 1.0f; // Extended prediction for survival
    NearFutureSample.Flags = EPoseSearchTrajectoryFlags::Position | EPoseSearchTrajectoryFlags::VelocityDirection;
    NearFutureSample.Weight = 0.8f;
    TrajectoryChannel->Samples.Add(NearFutureSample);
    
    Channels.Add(TrajectoryChannel);
    
    // Pose Channel - Foot and body positioning
    UPoseSearchFeatureChannel_Pose* PoseChannel = NewObject<UPoseSearchFeatureChannel_Pose>(this);
    PoseChannel->Weight = 0.8f;
    
    // Left foot for terrain adaptation
    FPoseSearchBone LeftFoot;
    LeftFoot.Reference = FBoneReference(TEXT("foot_l"));
    LeftFoot.Flags = EPoseSearchBoneFlags::Position | EPoseSearchBoneFlags::Velocity;
    LeftFoot.Weight = 1.0f;
    PoseChannel->SampledBones.Add(LeftFoot);
    
    // Right foot for terrain adaptation
    FPoseSearchBone RightFoot;
    RightFoot.Reference = FBoneReference(TEXT("foot_r"));
    RightFoot.Flags = EPoseSearchBoneFlags::Position | EPoseSearchBoneFlags::Velocity;
    RightFoot.Weight = 1.0f;
    PoseChannel->SampledBones.Add(RightFoot);
    
    // Spine for posture and fear response
    FPoseSearchBone Spine;
    Spine.Reference = FBoneReference(TEXT("spine_02"));
    Spine.Flags = EPoseSearchBoneFlags::Rotation | EPoseSearchBoneFlags::Velocity;
    Spine.Weight = 0.6f;
    PoseChannel->SampledBones.Add(Spine);
    
    Channels.Add(PoseChannel);
}

void UPoseSearchSchema_PlayerLocomotion::SetupFearChannels()
{
    // Velocity Channel for fear-influenced speed changes
    UPoseSearchFeatureChannel_Velocity* VelocityChannel = NewObject<UPoseSearchFeatureChannel_Velocity>(this);
    VelocityChannel->Weight = FearInfluenceWeight;
    VelocityChannel->Bone = FBoneReference(TEXT("pelvis"));
    VelocityChannel->bUseCharacterSpaceVelocities = true;
    
    Channels.Add(VelocityChannel);
    
    // Position Channel for head movement (looking around nervously)
    UPoseSearchFeatureChannel_Position* HeadPositionChannel = NewObject<UPoseSearchFeatureChannel_Position>(this);
    HeadPositionChannel->Weight = FearInfluenceWeight * 0.7f;
    HeadPositionChannel->Bone = FBoneReference(TEXT("head"));
    HeadPositionChannel->OriginBone = FBoneReference(TEXT("spine_03"));
    HeadPositionChannel->ComponentStripping = EPoseSearchComponentStripping::None;
    
    Channels.Add(HeadPositionChannel);
}

void UPoseSearchSchema_PlayerLocomotion::SetupTerrainChannels()
{
    // Additional position channels for terrain adaptation
    UPoseSearchFeatureChannel_Position* LeftFootPosition = NewObject<UPoseSearchFeatureChannel_Position>(this);
    LeftFootPosition->Weight = TerrainAdaptationWeight;
    LeftFootPosition->Bone = FBoneReference(TEXT("foot_l"));
    LeftFootPosition->OriginBone = FBoneReference(TEXT("pelvis"));
    LeftFootPosition->ComponentStripping = EPoseSearchComponentStripping::None;
    
    Channels.Add(LeftFootPosition);
    
    UPoseSearchFeatureChannel_Position* RightFootPosition = NewObject<UPoseSearchFeatureChannel_Position>(this);
    RightFootPosition->Weight = TerrainAdaptationWeight;
    RightFootPosition->Bone = FBoneReference(TEXT("foot_r"));
    RightFootPosition->OriginBone = FBoneReference(TEXT("pelvis"));
    RightFootPosition->ComponentStripping = EPoseSearchComponentStripping::None;
    
    Channels.Add(RightFootPosition);
}