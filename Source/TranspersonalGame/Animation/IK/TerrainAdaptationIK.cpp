#include "TerrainAdaptationIK.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UTerrainAdaptationIK::UTerrainAdaptationIK()
{
    TraceDistance = 100.0f;
    InterpSpeed = 15.0f;
    MaxFootOffset = 50.0f;
    HipAdjustmentSpeed = 10.0f;
    TraceChannel = ECC_WorldStatic;
    bIKEnabled = true;
    
    HipOffset = 0.0f;
    CurrentHipOffset = 0.0f;
    LastUpdateTime = 0.0f;
    LastOwnerLocation = FVector::ZeroVector;
}

void UTerrainAdaptationIK::UpdateTerrainIK(USkeletalMeshComponent* SkeletalMesh, float DeltaTime)
{
    if (!bIKEnabled || !SkeletalMesh || !SkeletalMesh->GetWorld())
    {
        return;
    }
    
    // Otimização: só atualizar se o personagem se moveu significativamente
    FVector CurrentLocation = SkeletalMesh->GetComponentLocation();
    float DistanceMoved = FVector::Dist(CurrentLocation, LastOwnerLocation);
    
    if (DistanceMoved < 5.0f && (GetWorld()->GetTimeSeconds() - LastUpdateTime) < 0.1f)
    {
        return; // Skip update para performance
    }
    
    LastOwnerLocation = CurrentLocation;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Calcular IK para ambos os pés
    LeftFootIK = CalculateFootIK(SkeletalMesh, TEXT("foot_l"), TEXT("thigh_l"), DeltaTime);
    RightFootIK = CalculateFootIK(SkeletalMesh, TEXT("foot_r"), TEXT("thigh_r"), DeltaTime);
    
    // Ajustar hip offset baseado nos pés
    UpdateHipOffset(LeftFootIK.DistanceFromGround, RightFootIK.DistanceFromGround, DeltaTime);
}

FFootIKData UTerrainAdaptationIK::CalculateFootIK(USkeletalMeshComponent* SkeletalMesh, 
                                                  const FName& FootBoneName, 
                                                  const FName& ThighBoneName,
                                                  float DeltaTime)
{
    FFootIKData FootData;
    
    if (!SkeletalMesh)
    {
        return FootData;
    }
    
    UWorld* World = SkeletalMesh->GetWorld();
    if (!World)
    {
        return FootData;
    }
    
    // Obter posição do pé no mundo
    FVector FootWorldLocation = SkeletalMesh->GetBoneLocation(FootBoneName, EBoneSpaceTransform::WorldSpace);
    FVector ThighWorldLocation = SkeletalMesh->GetBoneLocation(ThighBoneName, EBoneSpaceTransform::WorldSpace);
    
    // Trace para encontrar o chão
    FVector TraceStart = FootWorldLocation + FVector(0, 0, 50.0f);
    FHitResult HitResult = PerformFootTrace(TraceStart, World);
    
    if (HitResult.bBlockingHit)
    {
        // Calcular distância e offset
        float DistanceToGround = FootWorldLocation.Z - HitResult.Location.Z;
        FootData.DistanceFromGround = DistanceToGround;
        
        // Determinar se o pé deve ser ajustado
        if (FMath::Abs(DistanceToGround) < MaxFootOffset)
        {
            FootData.IKAlpha = CalculateFootAlpha(DistanceToGround);
            FootData.FootLocation = FMath::VInterpTo(FootData.FootLocation, 
                                                    HitResult.Location, 
                                                    DeltaTime, 
                                                    InterpSpeed);
            FootData.FootRotation = CalculateFootRotation(HitResult.Normal);
            FootData.bIsPlanted = true;
        }
        else
        {
            // Pé muito longe do chão - não aplicar IK
            FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaTime, InterpSpeed);
            FootData.bIsPlanted = false;
        }
    }
    else
    {
        // Não encontrou chão - desabilitar IK
        FootData.IKAlpha = FMath::FInterpTo(FootData.IKAlpha, 0.0f, DeltaTime, InterpSpeed);
        FootData.bIsPlanted = false;
    }
    
    return FootData;
}

FHitResult UTerrainAdaptationIK::PerformFootTrace(const FVector& StartLocation, UWorld* World) const
{
    FHitResult HitResult;
    FVector EndLocation = StartLocation - FVector(0, 0, TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = true;
    
    World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        TraceChannel,
        QueryParams
    );
    
    // Debug drawing (apenas em desenvolvimento)
    #if WITH_EDITOR
    if (CVarShowDebugIK.GetValueOnGameThread())
    {
        DrawDebugLine(World, StartLocation, EndLocation, 
                     HitResult.bBlockingHit ? FColor::Green : FColor::Red, 
                     false, 0.1f, 0, 1.0f);
        
        if (HitResult.bBlockingHit)
        {
            DrawDebugSphere(World, HitResult.Location, 5.0f, 8, FColor::Yellow, false, 0.1f);
        }
    }
    #endif
    
    return HitResult;
}

float UTerrainAdaptationIK::CalculateFootAlpha(float DistanceFromGround) const
{
    // Curva suave para transição do IK
    float NormalizedDistance = FMath::Abs(DistanceFromGround) / MaxFootOffset;
    return FMath::Clamp(1.0f - NormalizedDistance, 0.0f, 1.0f);
}

FRotator UTerrainAdaptationIK::CalculateFootRotation(const FVector& SurfaceNormal) const
{
    // Calcular rotação do pé baseada na normal da superfície
    FVector ForwardVector = FVector::ForwardVector;
    FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, SurfaceNormal);
}

void UTerrainAdaptationIK::UpdateHipOffset(float LeftFootDistance, float RightFootDistance, float DeltaTime)
{
    // Calcular offset do hip baseado no pé mais baixo
    float TargetHipOffset = FMath::Min(LeftFootDistance, RightFootDistance);
    
    // Limitar o offset para evitar poses estranhas
    TargetHipOffset = FMath::Clamp(TargetHipOffset, -MaxFootOffset * 0.5f, MaxFootOffset * 0.5f);
    
    // Suavizar a transição
    HipOffset = FMath::FInterpTo(HipOffset, TargetHipOffset, DeltaTime, HipAdjustmentSpeed);
    CurrentHipOffset = HipOffset;
}