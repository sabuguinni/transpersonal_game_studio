#include "TerrainAdaptationIK.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UTerrainAdaptationIK::UTerrainAdaptationIK()
{
    // Configurações padrão para IK de terreno
    MaxFootOffset = 50.0f;
    FootTraceDistance = 100.0f;
    IKInterpSpeed = 15.0f;
    HipAdjustmentRatio = 0.5f;
    
    // Configurar nomes dos bones
    LeftFootBoneName = "foot_l";
    RightFootBoneName = "foot_r";
    LeftIKBoneName = "ik_foot_l";
    RightIKBoneName = "ik_foot_r";
    HipBoneName = "pelvis";
    
    bEnableDebugDrawing = false;
}

void UTerrainAdaptationIK::UpdateFootIK(USkeletalMeshComponent* SkeletalMesh, float DeltaTime)
{
    if (!SkeletalMesh || !SkeletalMesh->GetWorld())
    {
        return;
    }

    // Atualizar IK para cada pé
    UpdateSingleFootIK(SkeletalMesh, LeftFootBoneName, LeftIKBoneName, LeftFootIKOffset, DeltaTime);
    UpdateSingleFootIK(SkeletalMesh, RightFootBoneName, RightIKBoneName, RightFootIKOffset, DeltaTime);
    
    // Ajustar quadril baseado na diferença entre os pés
    UpdateHipAdjustment(SkeletalMesh, DeltaTime);
}

void UTerrainAdaptationIK::UpdateSingleFootIK(USkeletalMeshComponent* SkeletalMesh, 
    const FName& FootBoneName, const FName& IKBoneName, 
    float& FootOffset, float DeltaTime)
{
    // Obter posição do pé no mundo
    FVector FootWorldLocation = SkeletalMesh->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
    
    // Configurar trace para o chão
    FVector TraceStart = FootWorldLocation + FVector(0, 0, 50.0f);
    FVector TraceEnd = FootWorldLocation - FVector(0, 0, FootTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(SkeletalMesh->GetOwner());
    
    // Executar trace
    bool bHit = SkeletalMesh->GetWorld()->LineTraceSingleByChannel(
        HitResult, 
        TraceStart, 
        TraceEnd, 
        ECC_WorldStatic, 
        QueryParams
    );
    
    float TargetOffset = 0.0f;
    
    if (bHit)
    {
        // Calcular offset necessário
        float GroundHeight = HitResult.Location.Z;
        float FootHeight = FootWorldLocation.Z;
        TargetOffset = FMath::Clamp(GroundHeight - FootHeight, -MaxFootOffset, MaxFootOffset);
        
        // Aplicar rotação baseada na normal do terreno
        FRotator TargetRotation = UKismetMathLibrary::MakeRotFromZX(HitResult.Normal, SkeletalMesh->GetForwardVector());
        // Aqui aplicaríamos a rotação ao IK bone se necessário
    }
    
    // Interpolar suavemente para o offset target
    FootOffset = FMath::FInterpTo(FootOffset, TargetOffset, DeltaTime, IKInterpSpeed);
    
    // Debug drawing
    if (bEnableDebugDrawing)
    {
        DrawDebugLine(SkeletalMesh->GetWorld(), TraceStart, TraceEnd, 
            bHit ? FColor::Green : FColor::Red, false, 0.1f);
        
        if (bHit)
        {
            DrawDebugSphere(SkeletalMesh->GetWorld(), HitResult.Location, 5.0f, 8, FColor::Yellow, false, 0.1f);
        }
    }
}

void UTerrainAdaptationIK::UpdateHipAdjustment(USkeletalMeshComponent* SkeletalMesh, float DeltaTime)
{
    // Calcular ajuste do quadril baseado na diferença entre os pés
    float FootDifference = LeftFootIKOffset - RightFootIKOffset;
    float TargetHipOffset = FootDifference * HipAdjustmentRatio;
    
    // Interpolar suavemente
    HipIKOffset = FMath::FInterpTo(HipIKOffset, TargetHipOffset, DeltaTime, IKInterpSpeed * 0.7f);
}

FVector UTerrainAdaptationIK::GetFootIKLocation(const FName& FootBoneName, USkeletalMeshComponent* SkeletalMesh)
{
    if (FootBoneName == LeftIKBoneName)
    {
        return FVector(0, 0, LeftFootIKOffset);
    }
    else if (FootBoneName == RightIKBoneName)
    {
        return FVector(0, 0, RightFootIKOffset);
    }
    
    return FVector::ZeroVector;
}

float UTerrainAdaptationIK::GetHipOffset() const
{
    return HipIKOffset;
}

void UTerrainAdaptationIK::SetDebugDrawing(bool bEnabled)
{
    bEnableDebugDrawing = bEnabled;
}