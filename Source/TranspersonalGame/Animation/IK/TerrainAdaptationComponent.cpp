#include "TerrainAdaptationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "../Core/TranspersonalAnimationSubsystem.h"

UTerrainAdaptationComponent::UTerrainAdaptationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Configurações padrão
    bIKEnabled = true;
    bDebugIK = false;
    TerrainAdaptationWeight = 1.0f;
    CurrentHipOffset = 0.0f;
    TargetHipOffset = 0.0f;
    
    // Nomes padrão dos bones
    LeftFootBoneName = FName("foot_l");
    RightFootBoneName = FName("foot_r");
    HipBoneName = FName("pelvis");
    
    // Configurações padrão de IK
    IKSettings = FTerrainAdaptationSettings();
}

void UTerrainAdaptationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontrar o SkeletalMeshComponent
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        OwnerMesh = Character->GetMesh();
    }
    else if (APawn* Pawn = Cast<APawn>(GetOwner()))
    {
        OwnerMesh = Pawn->FindComponentByClass<USkeletalMeshComponent>();
    }
    else
    {
        OwnerMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    if (!OwnerMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainAdaptationComponent: No SkeletalMeshComponent found on %s"), *GetOwner()->GetName());
        bIKEnabled = false;
        return;
    }
    
    // Obter subsistema de animação
    if (UWorld* World = GetWorld())
    {
        AnimationSubsystem = World->GetSubsystem<UTranspersonalAnimationSubsystem>();
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainAdaptationComponent initialized on %s"), *GetOwner()->GetName());
}

void UTerrainAdaptationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIKEnabled || !OwnerMesh)
    {
        return;
    }
    
    UpdateFootIK(DeltaTime);
    UpdateHipAdjustment(DeltaTime);
    
    if (bDebugIK)
    {
        DrawDebugIK();
    }
}

void UTerrainAdaptationComponent::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMesh || !OwnerMesh->GetSkeletalMeshAsset())
    {
        return;
    }
    
    // Atualizar IK do pé esquerdo
    FVector LeftFootWorldLocation = GetFootWorldLocation(LeftFootBoneName);
    LeftFootIK = CalculateFootIK(LeftFootBoneName, LeftFootWorldLocation);
    
    // Atualizar IK do pé direito
    FVector RightFootWorldLocation = GetFootWorldLocation(RightFootBoneName);
    RightFootIK = CalculateFootIK(RightFootBoneName, RightFootWorldLocation);
    
    // Interpolar valores de IK
    float InterpSpeed = IKSettings.IKInterpSpeed * DeltaTime * TerrainAdaptationWeight;
    
    // Smooth interpolation para posições
    LeftFootIK.FootLocation = FMath::VInterpTo(LeftFootIK.FootLocation, LeftFootIK.FootLocation, DeltaTime, InterpSpeed);
    RightFootIK.FootLocation = FMath::VInterpTo(RightFootIK.FootLocation, RightFootIK.FootLocation, DeltaTime, InterpSpeed);
    
    // Smooth interpolation para rotações
    if (IKSettings.bEnableFootRotation)
    {
        LeftFootIK.FootRotation = FMath::RInterpTo(LeftFootIK.FootRotation, LeftFootIK.FootRotation, DeltaTime, InterpSpeed);
        RightFootIK.FootRotation = FMath::RInterpTo(RightFootIK.FootRotation, RightFootIK.FootRotation, DeltaTime, InterpSpeed);
    }
    
    // Calcular hip offset se habilitado
    if (IKSettings.bEnableHipAdjustment)
    {
        float LeftFootDistance = FMath::Abs(LeftFootIK.DistanceFromGround);
        float RightFootDistance = FMath::Abs(RightFootIK.DistanceFromGround);
        
        // Usar o menor offset para manter ambos os pés no chão
        float MinFootDistance = FMath::Min(LeftFootDistance, RightFootDistance);
        TargetHipOffset = FMath::Clamp(-MinFootDistance, -IKSettings.MaxHipAdjustment, IKSettings.MaxHipAdjustment);
    }
}

FFootIKData UTerrainAdaptationComponent::CalculateFootIK(const FName& FootBoneName, const FVector& FootLocation)
{
    FFootIKData FootData;
    FootData.FootLocation = FootLocation;
    FootData.FootRotation = FRotator::ZeroRotator;
    FootData.IKAlpha = 0.0f;
    FootData.DistanceFromGround = 0.0f;
    FootData.TerrainType = ETerrainType::Flat;
    
    if (!OwnerMesh || FootBoneName.IsNone())
    {
        return FootData;
    }
    
    // Configurar trace para o solo
    FVector TraceStart = FootLocation + FVector(0, 0, IKSettings.FootTraceDistance * 0.5f);
    FVector TraceEnd = FootLocation - FVector(0, 0, IKSettings.FootTraceDistance);
    
    // Configurar parâmetros do trace
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());
    
    FHitResult HitResult;
    bool bHit = UKismetSystemLibrary::SphereTraceSingle(
        GetWorld(),
        TraceStart,
        TraceEnd,
        IKSettings.FootRadius,
        UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
        false, // bTraceComplex
        ActorsToIgnore,
        bDebugIK ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
        HitResult,
        true // bIgnoreSelf
    );
    
    if (bHit)
    {
        // Calcular distância do pé ao solo
        float DistanceToGround = FootLocation.Z - HitResult.Location.Z;
        FootData.DistanceFromGround = DistanceToGround;
        
        // Aplicar IK apenas se dentro da distância máxima
        if (FMath::Abs(DistanceToGround) <= IKSettings.MaxIKDistance)
        {
            FootData.FootLocation = HitResult.Location;
            FootData.IKAlpha = FMath::Clamp(1.0f - (FMath::Abs(DistanceToGround) / IKSettings.MaxIKDistance), 0.0f, 1.0f);
            
            // Calcular rotação baseada na normal da superfície
            if (IKSettings.bEnableFootRotation)
            {
                FootData.FootRotation = CalculateFootRotationFromNormal(HitResult.Normal);
            }
            
            // Determinar tipo de terreno baseado na normal
            float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector)));
            
            if (SlopeAngle < 15.0f)
            {
                FootData.TerrainType = ETerrainType::Flat;
            }
            else if (SlopeAngle < 45.0f)
            {
                FootData.TerrainType = ETerrainType::Slope;
            }
            else
            {
                FootData.TerrainType = ETerrainType::Steep;
            }
        }
    }
    
    // Aplicar peso de adaptação ao terreno
    FootData.IKAlpha *= TerrainAdaptationWeight;
    
    return FootData;
}

void UTerrainAdaptationComponent::UpdateHipAdjustment(float DeltaTime)
{
    if (!IKSettings.bEnableHipAdjustment)
    {
        CurrentHipOffset = 0.0f;
        return;
    }
    
    // Interpolar suavemente para o offset alvo
    float InterpSpeed = IKSettings.HipAdjustmentSpeed * DeltaTime * TerrainAdaptationWeight;
    CurrentHipOffset = FMath::FInterpTo(CurrentHipOffset, TargetHipOffset, DeltaTime, InterpSpeed);
}

FVector UTerrainAdaptationComponent::GetFootWorldLocation(const FName& FootBoneName)
{
    if (!OwnerMesh || FootBoneName.IsNone())
    {
        return GetOwner()->GetActorLocation();
    }
    
    return OwnerMesh->GetBoneLocation(FootBoneName, EBoneSpaces::WorldSpace);
}

FRotator UTerrainAdaptationComponent::CalculateFootRotationFromNormal(const FVector& SurfaceNormal)
{
    // Calcular rotação para alinhar o pé com a superfície
    FVector ForwardVector = GetOwner()->GetActorForwardVector();
    FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
    
    return FRotationMatrix::MakeFromXZ(ForwardVector, SurfaceNormal).Rotator();
}

void UTerrainAdaptationComponent::DrawDebugIK()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Debug para pé esquerdo
    FVector LeftFootLocation = GetFootWorldLocation(LeftFootBoneName);
    DrawDebugSphere(GetWorld(), LeftFootLocation, 5.0f, 8, FColor::Blue, false, 0.0f, 0, 1.0f);
    
    if (LeftFootIK.IKAlpha > 0.0f)
    {
        DrawDebugSphere(GetWorld(), LeftFootIK.FootLocation, 4.0f, 8, FColor::Green, false, 0.0f, 0, 1.0f);
        DrawDebugLine(GetWorld(), LeftFootLocation, LeftFootIK.FootLocation, FColor::Yellow, false, 0.0f, 0, 1.0f);
    }
    
    // Debug para pé direito
    FVector RightFootLocation = GetFootWorldLocation(RightFootBoneName);
    DrawDebugSphere(GetWorld(), RightFootLocation, 5.0f, 8, FColor::Blue, false, 0.0f, 0, 1.0f);
    
    if (RightFootIK.IKAlpha > 0.0f)
    {
        DrawDebugSphere(GetWorld(), RightFootIK.FootLocation, 4.0f, 8, FColor::Green, false, 0.0f, 0, 1.0f);
        DrawDebugLine(GetWorld(), RightFootLocation, RightFootIK.FootLocation, FColor::Yellow, false, 0.0f, 0, 1.0f);
    }
    
    // Debug para hip offset
    if (IKSettings.bEnableHipAdjustment && FMath::Abs(CurrentHipOffset) > 0.1f)
    {
        FVector HipLocation = GetOwner()->GetActorLocation() + FVector(0, 0, CurrentHipOffset);
        DrawDebugSphere(GetWorld(), HipLocation, 8.0f, 8, FColor::Red, false, 0.0f, 0, 1.0f);
        
        FString OffsetText = FString::Printf(TEXT("Hip Offset: %.1f"), CurrentHipOffset);
        DrawDebugString(GetWorld(), HipLocation + FVector(0, 0, 20), OffsetText, nullptr, FColor::White, 0.0f);
    }
    
    // Debug de informações gerais
    FVector ActorLocation = GetOwner()->GetActorLocation();
    FString DebugText = FString::Printf(TEXT("IK Enabled: %s\nWeight: %.2f\nLeft Alpha: %.2f\nRight Alpha: %.2f"),
        bIKEnabled ? TEXT("True") : TEXT("False"),
        TerrainAdaptationWeight,
        LeftFootIK.IKAlpha,
        RightFootIK.IKAlpha);
    
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 100), DebugText, nullptr, FColor::White, 0.0f);
}

// === FUNÇÕES PÚBLICAS ===

void UTerrainAdaptationComponent::SetIKEnabled(bool bEnabled)
{
    bIKEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Reset valores quando desabilitado
        LeftFootIK = FFootIKData();
        RightFootIK = FFootIKData();
        CurrentHipOffset = 0.0f;
        TargetHipOffset = 0.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainAdaptationComponent IK %s on %s"), 
        bEnabled ? TEXT("enabled") : TEXT("disabled"), *GetOwner()->GetName());
}

void UTerrainAdaptationComponent::SetFootBoneNames(const FName& LeftFootBone, const FName& RightFootBone)
{
    LeftFootBoneName = LeftFootBone;
    RightFootBoneName = RightFootBone;
    
    UE_LOG(LogTemp, Log, TEXT("Set foot bone names: Left=%s, Right=%s"), 
        *LeftFootBoneName.ToString(), *RightFootBoneName.ToString());
}

void UTerrainAdaptationComponent::SetIKSettings(const FTerrainAdaptationSettings& NewSettings)
{
    IKSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Updated IK settings on %s"), *GetOwner()->GetName());
}

void UTerrainAdaptationComponent::SetTerrainAdaptationWeight(float Weight)
{
    TerrainAdaptationWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("Set terrain adaptation weight to %.2f on %s"), 
        TerrainAdaptationWeight, *GetOwner()->GetName());
}