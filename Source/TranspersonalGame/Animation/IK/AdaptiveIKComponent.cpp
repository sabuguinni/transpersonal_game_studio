#include "AdaptiveIKComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

UAdaptiveIKComponent::UAdaptiveIKComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Configurações padrão
    MaxGroundTraceDistance = 100.0f;
    TraceStartOffset = 20.0f;
    BaseIKInterpolationSpeed = 15.0f;
    MaxIKIntensity = 1.0f;
    VelocityThreshold = 800.0f;
    bOnlyApplyWhenGrounded = true;
    CurrentTerrainType = "Default";
    
    // Configuração padrão de terreno
    FTerrainAdaptationSettings DefaultSettings;
    DefaultSettings.GripMultiplier = 1.0f;
    DefaultSettings.AdaptationSpeed = 10.0f;
    DefaultSettings.SurfaceOffset = 0.0f;
    DefaultSettings.bApplySurfaceRotation = true;
    TerrainSettings.Add("Default", DefaultSettings);
    
    bShowDebugTraces = false;
    bShowIKPositions = false;
}

void UAdaptiveIKComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-detectar SkeletalMeshComponent se não foi definido
    if (!TargetMesh)
    {
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            TargetMesh = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>();
        }
        else
        {
            TargetMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
    
    if (!TargetMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdaptiveIKComponent: No SkeletalMeshComponent found on %s"), *GetOwner()->GetName());
        return;
    }
    
    // Inicializar posição anterior
    LastActorLocation = GetOwner()->GetActorLocation();
    
    UE_LOG(LogTemp, Log, TEXT("AdaptiveIKComponent initialized on %s with %d feet"), *GetOwner()->GetName(), FeetIKData.Num());
}

void UAdaptiveIKComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!TargetMesh || !TargetMesh->GetSkeletalMeshAsset())
    {
        return;
    }
    
    // Atualizar velocidade
    UpdateVelocity(DeltaTime);
    
    // Verificar se devemos aplicar IK baseado na velocidade
    float CurrentIKIntensity = GetCurrentIKIntensity();
    if (CurrentIKIntensity <= 0.01f)
    {
        return;
    }
    
    // Atualizar IK para cada pé
    for (FFootIKData& FootData : FeetIKData)
    {
        UpdateFootIK(FootData, DeltaTime);
    }
    
    // Debug visual se ativado
    if (bShowDebugTraces || bShowIKPositions)
    {
        DrawDebugInfo();
    }
}

void UAdaptiveIKComponent::UpdateFootIK(FFootIKData& FootData, float DeltaTime)
{
    if (FootData.FootBoneName == "None" || FootData.FootBoneName.IsNone())
    {
        return;
    }
    
    FVector GroundLocation;
    FVector SurfaceNormal;
    bool bGroundHit = TraceGroundForFoot(FootData, GroundLocation, SurfaceNormal);
    
    FootData.bIsGrounded = bGroundHit;
    FootData.SurfaceNormal = SurfaceNormal;
    
    if (bGroundHit)
    {
        // Calcular distância ao solo
        FVector FootWorldLocation = GetFootWorldLocation(FootData);
        FootData.GroundDistance = FVector::Dist(FootWorldLocation, GroundLocation);
        
        // Aplicar configurações de terreno
        FTerrainAdaptationSettings TerrainSettings = GetCurrentTerrainSettings();
        
        // Calcular nova posição de IK
        FVector TargetIKLocation = GroundLocation + SurfaceNormal * TerrainSettings.SurfaceOffset;
        
        // Interpolar suavemente
        float InterpSpeed = BaseIKInterpolationSpeed * TerrainSettings.AdaptationSpeed * GetCurrentIKIntensity();
        FootData.CurrentIKLocation = FMath::VInterpTo(FootData.CurrentIKLocation, TargetIKLocation, DeltaTime, InterpSpeed);
        
        // Calcular rotação baseada na normal da superfície se habilitado
        if (TerrainSettings.bApplySurfaceRotation)
        {
            FVector ForwardVector = GetOwner()->GetActorForwardVector();
            FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
            ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
            
            FRotator TargetRotation = FRotationMatrix::MakeFromXZ(ForwardVector, SurfaceNormal).Rotator();
            FootData.CurrentIKRotation = FMath::RInterpTo(FootData.CurrentIKRotation, TargetRotation, DeltaTime, InterpSpeed);
        }
    }
    else
    {
        // Não há solo - interpolar de volta à posição padrão
        FootData.GroundDistance = MaxGroundTraceDistance;
        FVector DefaultLocation = GetFootWorldLocation(FootData);
        FootData.CurrentIKLocation = FMath::VInterpTo(FootData.CurrentIKLocation, DefaultLocation, DeltaTime, BaseIKInterpolationSpeed);
        FootData.CurrentIKRotation = FMath::RInterpTo(FootData.CurrentIKRotation, FRotator::ZeroRotator, DeltaTime, BaseIKInterpolationSpeed);
    }
}

bool UAdaptiveIKComponent::TraceGroundForFoot(const FFootIKData& FootData, FVector& OutGroundLocation, FVector& OutSurfaceNormal) const
{
    if (!TargetMesh)
    {
        return false;
    }
    
    // Obter posição mundial do pé
    FVector FootWorldLocation = GetFootWorldLocation(FootData);
    
    // Configurar trace
    FVector TraceStart = FootWorldLocation + FVector(0, 0, TraceStartOffset);
    FVector TraceEnd = FootWorldLocation + FVector(0, 0, -MaxGroundTraceDistance);
    
    // Configurar parâmetros do trace
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());
    
    FHitResult HitResult;
    bool bHit = UKismetSystemLibrary::LineTraceSingle(
        GetWorld(),
        TraceStart,
        TraceEnd,
        UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
        false, // bTraceComplex
        ActorsToIgnore,
        bShowDebugTraces ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
        HitResult,
        true // bIgnoreSelf
    );
    
    if (bHit)
    {
        OutGroundLocation = HitResult.Location;
        OutSurfaceNormal = HitResult.Normal;
        return true;
    }
    
    OutGroundLocation = FVector::ZeroVector;
    OutSurfaceNormal = FVector::UpVector;
    return false;
}

FVector UAdaptiveIKComponent::GetFootWorldLocation(const FFootIKData& FootData) const
{
    if (!TargetMesh || FootData.FootBoneName.IsNone())
    {
        return GetOwner()->GetActorLocation();
    }
    
    // Obter posição do bone do pé
    FVector BoneLocation = TargetMesh->GetBoneLocation(FootData.FootBoneName, EBoneSpaces::WorldSpace);
    
    // Aplicar offset do pé
    FVector FootLocation = BoneLocation + GetOwner()->GetActorTransform().TransformVectorNoScale(FootData.FootOffset);
    
    return FootLocation;
}

FTerrainAdaptationSettings UAdaptiveIKComponent::GetCurrentTerrainSettings() const
{
    if (const FTerrainAdaptationSettings* Settings = TerrainSettings.Find(CurrentTerrainType))
    {
        return *Settings;
    }
    
    // Retornar configurações padrão se não encontrar
    if (const FTerrainAdaptationSettings* DefaultSettings = TerrainSettings.Find("Default"))
    {
        return *DefaultSettings;
    }
    
    // Fallback para configurações hardcoded
    FTerrainAdaptationSettings FallbackSettings;
    FallbackSettings.GripMultiplier = 1.0f;
    FallbackSettings.AdaptationSpeed = 10.0f;
    FallbackSettings.SurfaceOffset = 0.0f;
    FallbackSettings.bApplySurfaceRotation = true;
    return FallbackSettings;
}

void UAdaptiveIKComponent::UpdateVelocity(float DeltaTime)
{
    if (DeltaTime > 0.0f)
    {
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        FVector Velocity = (CurrentLocation - LastActorLocation) / DeltaTime;
        CurrentVelocity = Velocity.Size();
        LastActorLocation = CurrentLocation;
    }
}

float UAdaptiveIKComponent::GetCurrentIKIntensity() const
{
    if (CurrentVelocity >= VelocityThreshold)
    {
        return 0.0f;
    }
    
    // Interpolar intensidade baseada na velocidade
    float VelocityRatio = CurrentVelocity / VelocityThreshold;
    float Intensity = FMath::Lerp(MaxIKIntensity, 0.0f, VelocityRatio);
    
    return FMath::Clamp(Intensity, 0.0f, MaxIKIntensity);
}

void UAdaptiveIKComponent::DrawDebugInfo() const
{
    if (!GetWorld())
    {
        return;
    }
    
    for (const FFootIKData& FootData : FeetIKData)
    {
        if (FootData.FootBoneName.IsNone())
        {
            continue;
        }
        
        FVector FootWorldLocation = GetFootWorldLocation(FootData);
        
        if (bShowIKPositions)
        {
            // Mostrar posição atual do pé
            DrawDebugSphere(GetWorld(), FootWorldLocation, 5.0f, 8, FColor::Blue, false, 0.0f, 0, 1.0f);
            
            // Mostrar posição de IK se diferente
            if (!FootData.CurrentIKLocation.IsZero())
            {
                DrawDebugSphere(GetWorld(), FootData.CurrentIKLocation, 4.0f, 8, FColor::Green, false, 0.0f, 0, 1.0f);
                DrawDebugLine(GetWorld(), FootWorldLocation, FootData.CurrentIKLocation, FColor::Yellow, false, 0.0f, 0, 1.0f);
            }
            
            // Mostrar normal da superfície
            if (FootData.bIsGrounded)
            {
                DrawDebugLine(GetWorld(), FootData.CurrentIKLocation, 
                    FootData.CurrentIKLocation + FootData.SurfaceNormal * 20.0f, 
                    FColor::Red, false, 0.0f, 0, 2.0f);
            }
        }
    }
    
    // Mostrar informações de velocidade
    if (bShowDebugTraces)
    {
        FVector ActorLocation = GetOwner()->GetActorLocation();
        FString VelocityText = FString::Printf(TEXT("Velocity: %.1f\nIK Intensity: %.2f\nTerrain: %s"), 
            CurrentVelocity, GetCurrentIKIntensity(), *CurrentTerrainType.ToString());
        
        DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 100), VelocityText, nullptr, FColor::White, 0.0f);
    }
}

// === FUNÇÕES PÚBLICAS ===

void UAdaptiveIKComponent::AddFootForIK(FName FootBoneName, FVector FootOffset)
{
    // Verificar se já existe
    for (const FFootIKData& FootData : FeetIKData)
    {
        if (FootData.FootBoneName == FootBoneName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Foot %s already exists in IK system"), *FootBoneName.ToString());
            return;
        }
    }
    
    // Criar novo foot data
    FFootIKData NewFootData;
    NewFootData.FootBoneName = FootBoneName;
    NewFootData.FootOffset = FootOffset;
    NewFootData.CurrentIKLocation = FVector::ZeroVector;
    NewFootData.CurrentIKRotation = FRotator::ZeroRotator;
    NewFootData.bIsGrounded = false;
    NewFootData.GroundDistance = 0.0f;
    NewFootData.SurfaceNormal = FVector::UpVector;
    
    FeetIKData.Add(NewFootData);
    
    UE_LOG(LogTemp, Log, TEXT("Added foot %s to IK system"), *FootBoneName.ToString());
}

void UAdaptiveIKComponent::RemoveFootFromIK(FName FootBoneName)
{
    for (int32 i = FeetIKData.Num() - 1; i >= 0; i--)
    {
        if (FeetIKData[i].FootBoneName == FootBoneName)
        {
            FeetIKData.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Removed foot %s from IK system"), *FootBoneName.ToString());
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Foot %s not found in IK system"), *FootBoneName.ToString());
}

FFootIKData UAdaptiveIKComponent::GetFootIKData(FName FootBoneName) const
{
    for (const FFootIKData& FootData : FeetIKData)
    {
        if (FootData.FootBoneName == FootBoneName)
        {
            return FootData;
        }
    }
    
    // Retornar dados vazios se não encontrar
    FFootIKData EmptyData;
    return EmptyData;
}

void UAdaptiveIKComponent::SetCurrentTerrainType(FName TerrainType)
{
    if (CurrentTerrainType != TerrainType)
    {
        CurrentTerrainType = TerrainType;
        UE_LOG(LogTemp, Log, TEXT("Changed terrain type to %s"), *TerrainType.ToString());
    }
}

void UAdaptiveIKComponent::RecalculateAllFootIK()
{
    // Força recálculo imediato para todos os pés
    for (FFootIKData& FootData : FeetIKData)
    {
        FVector GroundLocation;
        FVector SurfaceNormal;
        bool bGroundHit = TraceGroundForFoot(FootData, GroundLocation, SurfaceNormal);
        
        if (bGroundHit)
        {
            FTerrainAdaptationSettings TerrainSettings = GetCurrentTerrainSettings();
            FootData.CurrentIKLocation = GroundLocation + SurfaceNormal * TerrainSettings.SurfaceOffset;
            FootData.bIsGrounded = true;
            FootData.SurfaceNormal = SurfaceNormal;
            
            if (TerrainSettings.bApplySurfaceRotation)
            {
                FVector ForwardVector = GetOwner()->GetActorForwardVector();
                FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
                ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
                FootData.CurrentIKRotation = FRotationMatrix::MakeFromXZ(ForwardVector, SurfaceNormal).Rotator();
            }
        }
        else
        {
            FootData.CurrentIKLocation = GetFootWorldLocation(FootData);
            FootData.CurrentIKRotation = FRotator::ZeroRotator;
            FootData.bIsGrounded = false;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Recalculated IK for all %d feet"), FeetIKData.Num());
}