#include "PoseSearchSchema_PlayerLocomotion.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "PoseSearch/PoseSearchDatabase.h"

UPoseSearchSchema_PlayerLocomotion::UPoseSearchSchema_PlayerLocomotion()
{
    // Configuração base para locomoção do paleontologista
    SampleRate = 30.0f; // 30fps para performance otimizada
    DataPreprocessor = EPoseSearchDataPreprocessor::Normalize;
    
    // Configurar canais para Motion Matching
    SetupTrajectoryChannel();
    SetupPoseChannel();
    SetupVelocityChannel();
}

void UPoseSearchSchema_PlayerLocomotion::SetupTrajectoryChannel()
{
    // Canal de trajetória para movimento do jogador
    FPoseSearchTrajectoryChannel TrajectoryChannel;
    TrajectoryChannel.Weight = 1.0f;
    
    // Amostras de trajetória: passado, presente e futuro
    // Passado: -0.5s, -0.25s
    // Presente: 0.0s
    // Futuro: +0.25s, +0.5s, +1.0s
    
    FPoseSearchTrajectorySample Sample;
    
    // Passado
    Sample.Offset = -0.5f;
    Sample.Flags = EPoseSearchTrajectoryFlags::Position | EPoseSearchTrajectoryFlags::Velocity;
    Sample.Weight = 0.8f;
    TrajectoryChannel.Samples.Add(Sample);
    
    Sample.Offset = -0.25f;
    Sample.Weight = 0.9f;
    TrajectoryChannel.Samples.Add(Sample);
    
    // Presente
    Sample.Offset = 0.0f;
    Sample.Weight = 1.0f;
    TrajectoryChannel.Samples.Add(Sample);
    
    // Futuro
    Sample.Offset = 0.25f;
    Sample.Weight = 1.0f;
    TrajectoryChannel.Samples.Add(Sample);
    
    Sample.Offset = 0.5f;
    Sample.Weight = 0.9f;
    TrajectoryChannel.Samples.Add(Sample);
    
    Sample.Offset = 1.0f;
    Sample.Weight = 0.7f;
    TrajectoryChannel.Samples.Add(Sample);
    
    // Adicionar canal ao schema
    Channels.Add(MakeShared<FPoseSearchTrajectoryChannel>(TrajectoryChannel));
}

void UPoseSearchSchema_PlayerLocomotion::SetupPoseChannel()
{
    // Canal de pose para pés e quadril
    FPoseSearchPoseChannel PoseChannel;
    PoseChannel.Weight = 1.2f; // Peso maior para pose dos pés
    
    // Pé esquerdo
    FPoseSearchBone LeftFoot;
    LeftFoot.Reference = FBoneReference("foot_l");
    LeftFoot.Flags = EPoseSearchBoneFlags::Position | EPoseSearchBoneFlags::Velocity;
    LeftFoot.Weight = 1.0f;
    PoseChannel.SampledBones.Add(LeftFoot);
    
    // Pé direito
    FPoseSearchBone RightFoot;
    RightFoot.Reference = FBoneReference("foot_r");
    RightFoot.Flags = EPoseSearchBoneFlags::Position | EPoseSearchBoneFlags::Velocity;
    RightFoot.Weight = 1.0f;
    PoseChannel.SampledBones.Add(RightFoot);
    
    // Quadril para estabilidade
    FPoseSearchBone Pelvis;
    Pelvis.Reference = FBoneReference("pelvis");
    Pelvis.Flags = EPoseSearchBoneFlags::Position | EPoseSearchBoneFlags::Rotation;
    Pelvis.Weight = 0.8f;
    PoseChannel.SampledBones.Add(Pelvis);
    
    Channels.Add(MakeShared<FPoseSearchPoseChannel>(PoseChannel));
}

void UPoseSearchSchema_PlayerLocomotion::SetupVelocityChannel()
{
    // Canal de velocidade para movimento do root bone
    FPoseSearchVelocityChannel VelocityChannel;
    VelocityChannel.Bone = FBoneReference("root");
    VelocityChannel.Weight = 0.9f;
    VelocityChannel.bUseCharacterSpaceVelocities = true;
    VelocityChannel.bNormalize = false;
    
    Channels.Add(MakeShared<FPoseSearchVelocityChannel>(VelocityChannel));
}