#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Configurações padrão
    SearchRadius = 100.0f;
    BlendTime = 0.2f;
    MaxCandidates = 50;
    bEnableDebugDraw = false;
    UpdateFrequency = 30.0f; // 30 FPS
    LastUpdateTime = 0.0f;
    
    // Ossos padrão para tracking
    TrackedBones.Add(TEXT("pelvis"));
    TrackedBones.Add(TEXT("spine_01"));
    TrackedBones.Add(TEXT("spine_02"));
    TrackedBones.Add(TEXT("spine_03"));
    TrackedBones.Add(TEXT("head"));
    TrackedBones.Add(TEXT("upperarm_l"));
    TrackedBones.Add(TEXT("upperarm_r"));
    TrackedBones.Add(TEXT("lowerarm_l"));
    TrackedBones.Add(TEXT("lowerarm_r"));
    TrackedBones.Add(TEXT("thigh_l"));
    TrackedBones.Add(TEXT("thigh_r"));
    TrackedBones.Add(TEXT("calf_l"));
    TrackedBones.Add(TEXT("calf_r"));
    
    RootBone = TEXT("pelvis");
    LeftFootBone = TEXT("foot_l");
    RightFootBone = TEXT("foot_r");
    
    // Inicializar referências
    SkeletalMesh = nullptr;
    OwnerCharacter = nullptr;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Obter referências
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMesh = OwnerCharacter->GetMesh();
        UE_LOG(LogTemp, Log, TEXT("MotionMatching: Inicializado para %s"), *OwnerCharacter->GetName());
    }
    
    // Inicializar ossos tracked
    InitializeTrackedBones();
    
    // Construir base de dados se temos animações
    if (IdleAnimations.Num() > 0 || WalkAnimations.Num() > 0 || RunAnimations.Num() > 0)
    {
        BuildMotionDatabase();
    }
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !SkeletalMesh)
    {
        return;
    }
    
    // Controlar frequência de update
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime < (1.0f / UpdateFrequency))
    {
        return;
    }
    LastUpdateTime = CurrentTime;
    
    // Atualizar query baseado no estado atual
    UpdateQuery();
    
    // Encontrar melhor match se temos base de dados
    if (MotionDatabase.Num() > 0)
    {
        CurrentMotion = FindBestMatch(CurrentQuery);
    }
    
    // Debug visualization
    if (bEnableDebugDraw)
    {
        DrawDebugInfo();
    }
}

void UAnim_MotionMatchingComponent::UpdateQuery()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Obter velocidade atual
    FVector CurrentVelocity = OwnerCharacter->GetVelocity();
    CurrentQuery.DesiredVelocity = CurrentVelocity;
    
    // Calcular velocidade angular
    FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    // TODO: Calcular velocidade angular baseado na rotação anterior
    CurrentQuery.DesiredAngularVelocity = 0.0f;
    
    // Posição e rotação atuais
    CurrentQuery.CurrentPosition = OwnerCharacter->GetActorLocation();
    CurrentQuery.CurrentRotation = CurrentRotation;
    
    // Obter posições dos ossos se possível
    if (SkeletalMesh && SkeletalMesh->GetAnimInstance())
    {
        CurrentQuery.CurrentBonePositions.Empty();
        
        for (const FName& BoneName : TrackedBones)
        {
            FVector BoneLocation = SkeletalMesh->GetBoneLocation(BoneName, EBoneSpaces::ComponentSpace);
            CurrentQuery.CurrentBonePositions.Add(BoneLocation);
        }
    }
}

FAnim_MotionData UAnim_MotionMatchingComponent::FindBestMatch(const FAnim_MotionQuery& Query)
{
    if (MotionDatabase.Num() == 0)
    {
        return FAnim_MotionData();
    }
    
    float BestCost = FLT_MAX;
    int32 BestIndex = 0;
    
    // Limitar número de candidatos para performance
    int32 CandidatesToCheck = FMath::Min(MaxCandidates, MotionDatabase.Num());
    
    for (int32 i = 0; i < CandidatesToCheck; i++)
    {
        float Cost = CalculateMotionCost(MotionDatabase[i], Query);
        
        if (Cost < BestCost)
        {
            BestCost = Cost;
            BestIndex = i;
        }
    }
    
    // Log do melhor match encontrado
    if (bEnableDebugDraw)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("MotionMatching: Melhor match com custo %.2f"), BestCost);
    }
    
    return MotionDatabase[BestIndex];
}

float UAnim_MotionMatchingComponent::CalculateMotionCost(const FAnim_MotionData& Motion, const FAnim_MotionQuery& Query) const
{
    float VelocityCost = CalculateVelocityCost(Motion, Query);
    float AngularCost = CalculateAngularCost(Motion, Query);
    float PoseCost = CalculatePoseCost(Motion, Query);
    
    // Combinar custos com pesos
    float TotalCost = (VelocityCost * Query.VelocityWeight) +
                      (AngularCost * Query.AngularWeight) +
                      (PoseCost * Query.PoseWeight);
    
    return TotalCost;
}

float UAnim_MotionMatchingComponent::CalculateVelocityCost(const FAnim_MotionData& Motion, const FAnim_MotionQuery& Query) const
{
    FVector VelocityDiff = Motion.RootMotionVelocity - Query.DesiredVelocity;
    return VelocityDiff.SizeSquared();
}

float UAnim_MotionMatchingComponent::CalculateAngularCost(const FAnim_MotionData& Motion, const FAnim_MotionQuery& Query) const
{
    float AngularDiff = Motion.AngularVelocity - Query.DesiredAngularVelocity;
    return AngularDiff * AngularDiff;
}

float UAnim_MotionMatchingComponent::CalculatePoseCost(const FAnim_MotionData& Motion, const FAnim_MotionQuery& Query) const
{
    if (Motion.BonePositions.Num() != Query.CurrentBonePositions.Num())
    {
        return 1000.0f; // Penalidade alta para incompatibilidade
    }
    
    float TotalCost = 0.0f;
    
    for (int32 i = 0; i < Motion.BonePositions.Num(); i++)
    {
        FVector Diff = Motion.BonePositions[i] - Query.CurrentBonePositions[i];
        TotalCost += Diff.SizeSquared();
    }
    
    return TotalCost / Motion.BonePositions.Num(); // Média
}

void UAnim_MotionMatchingComponent::BuildMotionDatabase()
{
    ClearDatabase();
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatching: A construir base de dados..."));
    
    // Adicionar animações idle
    for (UAnimSequence* Anim : IdleAnimations)
    {
        if (IsValidAnimation(Anim))
        {
            AddAnimationToDatabase(Anim, 10.0f); // Sample rate mais baixo para idle
        }
    }
    
    // Adicionar animações de caminhada
    for (UAnimSequence* Anim : WalkAnimations)
    {
        if (IsValidAnimation(Anim))
        {
            AddAnimationToDatabase(Anim, 30.0f);
        }
    }
    
    // Adicionar animações de corrida
    for (UAnimSequence* Anim : RunAnimations)
    {
        if (IsValidAnimation(Anim))
        {
            AddAnimationToDatabase(Anim, 30.0f);
        }
    }
    
    // Adicionar animações de rotação
    for (UAnimSequence* Anim : TurnAnimations)
    {
        if (IsValidAnimation(Anim))
        {
            AddAnimationToDatabase(Anim, 20.0f);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatching: Base de dados construída com %d entradas"), MotionDatabase.Num());
}

void UAnim_MotionMatchingComponent::AddAnimationToDatabase(UAnimSequence* Animation, float SampleRate)
{
    if (!IsValidAnimation(Animation))
    {
        return;
    }
    
    float AnimLength = Animation->GetPlayLength();
    float SampleInterval = 1.0f / SampleRate;
    
    for (float Time = 0.0f; Time < AnimLength; Time += SampleInterval)
    {
        FAnim_MotionData MotionData;
        ExtractMotionData(Animation, Time, MotionData);
        MotionDatabase.Add(MotionData);
    }
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatching: Adicionada animação %s com %d samples"), 
           *Animation->GetName(), FMath::CeilToInt(AnimLength * SampleRate));
}

void UAnim_MotionMatchingComponent::ExtractMotionData(UAnimSequence* Animation, float Time, FAnim_MotionData& OutData)
{
    if (!Animation)
    {
        return;
    }
    
    OutData.AnimSequence = Animation;
    OutData.StartTime = Time;
    OutData.EndTime = Time + (1.0f / UpdateFrequency); // Próximo frame
    
    // Extrair root motion (simplificado)
    // TODO: Implementar extração real de root motion
    OutData.RootMotionVelocity = FVector(100.0f, 0.0f, 0.0f); // Placeholder
    OutData.AngularVelocity = 0.0f;
    
    // Extrair posições dos ossos
    GetBonePositions(Animation, Time, OutData.BonePositions);
    GetBoneVelocities(Animation, Time, OutData.BoneVelocities);
    
    OutData.Weight = 1.0f;
}

void UAnim_MotionMatchingComponent::GetBonePositions(UAnimSequence* Animation, float Time, TArray<FVector>& OutPositions)
{
    OutPositions.Empty();
    
    // TODO: Implementar extração real de posições dos ossos da animação
    // Por agora, usar posições placeholder
    for (int32 i = 0; i < TrackedBones.Num(); i++)
    {
        OutPositions.Add(FVector(i * 10.0f, 0.0f, 0.0f));
    }
}

void UAnim_MotionMatchingComponent::GetBoneVelocities(UAnimSequence* Animation, float Time, TArray<FVector>& OutVelocities)
{
    OutVelocities.Empty();
    
    // TODO: Implementar cálculo de velocidades dos ossos
    // Por agora, usar velocidades zero
    for (int32 i = 0; i < TrackedBones.Num(); i++)
    {
        OutVelocities.Add(FVector::ZeroVector);
    }
}

void UAnim_MotionMatchingComponent::ClearDatabase()
{
    MotionDatabase.Empty();
    UE_LOG(LogTemp, Log, TEXT("MotionMatching: Base de dados limpa"));
}

void UAnim_MotionMatchingComponent::SetDesiredMovement(FVector Velocity, float AngularVelocity)
{
    CurrentQuery.DesiredVelocity = Velocity;
    CurrentQuery.DesiredAngularVelocity = AngularVelocity;
}

void UAnim_MotionMatchingComponent::EnableDebugVisualization(bool bEnable)
{
    bEnableDebugDraw = bEnable;
    UE_LOG(LogTemp, Log, TEXT("MotionMatching: Debug visualization %s"), 
           bEnable ? TEXT("ativado") : TEXT("desativado"));
}

void UAnim_MotionMatchingComponent::DrawDebugInfo()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    FVector ActorLocation = OwnerCharacter->GetActorLocation();
    
    // Desenhar velocidade desejada
    DrawDebugLine(GetWorld(), ActorLocation, ActorLocation + CurrentQuery.DesiredVelocity, 
                  FColor::Green, false, 0.1f, 0, 2.0f);
    
    // Desenhar informações de texto
    FString DebugText = FString::Printf(TEXT("Motion DB: %d entries\nVelocity: %.1f\nBest Match: %s"), 
                                       MotionDatabase.Num(),
                                       CurrentQuery.DesiredVelocity.Size(),
                                       CurrentMotion.AnimSequence ? *CurrentMotion.AnimSequence->GetName() : TEXT("None"));
    
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 200), DebugText, 
                   nullptr, FColor::White, 0.1f);
}

void UAnim_MotionMatchingComponent::InitializeTrackedBones()
{
    if (!SkeletalMesh || !SkeletalMesh->GetSkeletalMeshAsset())
    {
        return;
    }
    
    // Verificar se os ossos existem no skeleton
    USkeleton* Skeleton = SkeletalMesh->GetSkeletalMeshAsset()->GetSkeleton();
    if (!Skeleton)
    {
        return;
    }
    
    // Filtrar ossos que realmente existem
    TArray<FName> ValidBones;
    for (const FName& BoneName : TrackedBones)
    {
        if (Skeleton->GetReferenceSkeleton().FindBoneIndex(BoneName) != INDEX_NONE)
        {
            ValidBones.Add(BoneName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("MotionMatching: Osso %s não encontrado no skeleton"), *BoneName.ToString());
        }
    }
    
    TrackedBones = ValidBones;
    UE_LOG(LogTemp, Log, TEXT("MotionMatching: Inicializados %d ossos para tracking"), TrackedBones.Num());
}

bool UAnim_MotionMatchingComponent::IsValidAnimation(UAnimSequence* Animation) const
{
    if (!Animation)
    {
        return false;
    }
    
    if (Animation->GetPlayLength() <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatching: Animação %s tem duração inválida"), *Animation->GetName());
        return false;
    }
    
    return true;
}