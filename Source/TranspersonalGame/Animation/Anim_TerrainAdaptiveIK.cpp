#include "Anim_TerrainAdaptiveIK.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UAnim_TerrainAdaptiveIK::UAnim_TerrainAdaptiveIK()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    bIKEnabled = true;
    LastUpdateTime = 0.0f;
    OwnerMeshComponent = nullptr;
    
    // Configurações padrão
    IKSettings = FAnim_TerrainIKSettings();
}

void UAnim_TerrainAdaptiveIK::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponent();
}

void UAnim_TerrainAdaptiveIK::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIKEnabled && OwnerMeshComponent && OwnerMeshComponent->GetAnimInstance())
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_TerrainAdaptiveIK::InitializeComponent()
{
    CacheOwnerMeshComponent();
    InitializeFootData();
}

void UAnim_TerrainAdaptiveIK::CacheOwnerMeshComponent()
{
    AActor* Owner = GetOwner();
    if (Owner)
    {
        OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!OwnerMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("UAnim_TerrainAdaptiveIK: No SkeletalMeshComponent found on owner %s"), *Owner->GetName());
        }
    }
}

void UAnim_TerrainAdaptiveIK::InitializeFootData()
{
    FootIKData.Empty();
    PreviousFootLocations.Empty();
    PreviousFootRotations.Empty();
    
    for (const FName& BoneName : IKSettings.FootBoneNames)
    {
        FootIKData.Add(BoneName, FAnim_FootIKData());
        PreviousFootLocations.Add(BoneName, FVector::ZeroVector);
        PreviousFootRotations.Add(BoneName, FRotator::ZeroRotator);
    }
}

void UAnim_TerrainAdaptiveIK::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMeshComponent || !OwnerMeshComponent->GetAnimInstance())
    {
        return;
    }
    
    for (const FName& BoneName : IKSettings.FootBoneNames)
    {
        FAnim_FootIKData NewFootData = CalculateFootIK(BoneName, DeltaTime);
        FootIKData.Add(BoneName, NewFootData);
        
        if (IKSettings.bEnableDebugDraw)
        {
            FVector TraceStart = OwnerMeshComponent->GetBoneLocation(BoneName);
            FVector TraceEnd = TraceStart - FVector(0, 0, IKSettings.TraceDistance);
            DrawDebugInfo(BoneName, NewFootData, TraceStart, TraceEnd);
        }
    }
    
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

FAnim_FootIKData UAnim_TerrainAdaptiveIK::CalculateFootIK(const FName& BoneName, float DeltaTime)
{
    FAnim_FootIKData Result;
    
    if (!OwnerMeshComponent)
    {
        return Result;
    }
    
    // Obter posição actual do osso
    FVector BoneLocation = OwnerMeshComponent->GetBoneLocation(BoneName);
    
    // Realizar trace para o chão
    FVector HitLocation;
    FVector HitNormal;
    bool bHit = PerformGroundTrace(BoneLocation, HitLocation, HitNormal);
    
    if (bHit)
    {
        // Calcular nova posição do pé
        FVector TargetFootLocation = HitLocation + (HitNormal * IKSettings.FootOffset);
        
        // Interpolar suavemente
        FVector* PrevLocation = PreviousFootLocations.Find(BoneName);
        if (PrevLocation)
        {
            Result.FootLocation = FMath::VInterpTo(*PrevLocation, TargetFootLocation, DeltaTime, IKSettings.InterpSpeed);
        }
        else
        {
            Result.FootLocation = TargetFootLocation;
        }
        
        // Calcular rotação baseada na normal da superfície
        FRotator TargetFootRotation = CalculateFootRotationFromNormal(HitNormal);
        
        // Interpolar rotação
        FRotator* PrevRotation = PreviousFootRotations.Find(BoneName);
        if (PrevRotation)
        {
            Result.FootRotation = FMath::RInterpTo(*PrevRotation, TargetFootRotation, DeltaTime, IKSettings.InterpSpeed);
        }
        else
        {
            Result.FootRotation = TargetFootRotation;
        }
        
        // Calcular distância do chão e alpha do IK
        Result.DistanceFromGround = FVector::Dist(BoneLocation, HitLocation);
        Result.IKAlpha = FMath::Clamp(Result.DistanceFromGround / IKSettings.TraceDistance, 0.0f, 1.0f);
        
        // Actualizar cache
        PreviousFootLocations.Add(BoneName, Result.FootLocation);
        PreviousFootRotations.Add(BoneName, Result.FootRotation);
    }
    else
    {
        // Sem hit - desactivar IK gradualmente
        Result.IKAlpha = 0.0f;
        Result.FootLocation = BoneLocation;
        Result.FootRotation = FRotator::ZeroRotator;
        Result.DistanceFromGround = IKSettings.TraceDistance;
    }
    
    return Result;
}

bool UAnim_TerrainAdaptiveIK::PerformGroundTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!GetWorld())
    {
        return false;
    }
    
    FVector TraceStart = StartLocation + FVector(0, 0, 50.0f); // Começar ligeiramente acima
    FVector TraceEnd = StartLocation - FVector(0, 0, IKSettings.TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = false;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        return true;
    }
    
    OutHitLocation = FVector::ZeroVector;
    OutHitNormal = FVector::UpVector;
    return false;
}

FRotator UAnim_TerrainAdaptiveIK::CalculateFootRotationFromNormal(const FVector& SurfaceNormal)
{
    // Calcular rotação para alinhar o pé com a superfície
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
    
    return UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, SurfaceNormal);
}

void UAnim_TerrainAdaptiveIK::SetIKEnabled(bool bEnabled)
{
    bIKEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Limpar dados quando desactivado
        for (auto& Pair : FootIKData)
        {
            Pair.Value.IKAlpha = 0.0f;
        }
    }
}

void UAnim_TerrainAdaptiveIK::AddFootBone(const FName& BoneName)
{
    if (!IKSettings.FootBoneNames.Contains(BoneName))
    {
        IKSettings.FootBoneNames.Add(BoneName);
        FootIKData.Add(BoneName, FAnim_FootIKData());
        PreviousFootLocations.Add(BoneName, FVector::ZeroVector);
        PreviousFootRotations.Add(BoneName, FRotator::ZeroRotator);
    }
}

void UAnim_TerrainAdaptiveIK::RemoveFootBone(const FName& BoneName)
{
    IKSettings.FootBoneNames.Remove(BoneName);
    FootIKData.Remove(BoneName);
    PreviousFootLocations.Remove(BoneName);
    PreviousFootRotations.Remove(BoneName);
}

FAnim_FootIKData UAnim_TerrainAdaptiveIK::GetFootIKData(const FName& BoneName) const
{
    const FAnim_FootIKData* FoundData = FootIKData.Find(BoneName);
    return FoundData ? *FoundData : FAnim_FootIKData();
}

float UAnim_TerrainAdaptiveIK::GetFootIKAlpha(const FName& BoneName) const
{
    const FAnim_FootIKData* FoundData = FootIKData.Find(BoneName);
    return FoundData ? FoundData->IKAlpha : 0.0f;
}

FVector UAnim_TerrainAdaptiveIK::GetFootIKLocation(const FName& BoneName) const
{
    const FAnim_FootIKData* FoundData = FootIKData.Find(BoneName);
    return FoundData ? FoundData->FootLocation : FVector::ZeroVector;
}

FRotator UAnim_TerrainAdaptiveIK::GetFootIKRotation(const FName& BoneName) const
{
    const FAnim_FootIKData* FoundData = FootIKData.Find(BoneName);
    return FoundData ? FoundData->FootRotation : FRotator::ZeroRotator;
}

void UAnim_TerrainAdaptiveIK::DrawDebugInfo(const FName& BoneName, const FAnim_FootIKData& FootData, const FVector& TraceStart, const FVector& TraceEnd)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Linha de trace
    FColor TraceColor = FootData.IKAlpha > 0.0f ? FColor::Green : FColor::Red;
    DrawDebugLine(GetWorld(), TraceStart, TraceEnd, TraceColor, false, 0.0f, 0, 1.0f);
    
    // Posição do pé
    if (FootData.IKAlpha > 0.0f)
    {
        DrawDebugSphere(GetWorld(), FootData.FootLocation, 5.0f, 8, FColor::Blue, false, 0.0f, 0, 1.0f);
        
        // Seta para mostrar orientação
        FVector ArrowEnd = FootData.FootLocation + (FootData.FootRotation.Vector() * 20.0f);
        DrawDebugDirectionalArrow(GetWorld(), FootData.FootLocation, ArrowEnd, 5.0f, FColor::Yellow, false, 0.0f, 0, 1.0f);
    }
}