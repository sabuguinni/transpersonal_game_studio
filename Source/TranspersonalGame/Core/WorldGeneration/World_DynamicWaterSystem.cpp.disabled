#include "World_DynamicWaterSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/StaticMesh.h"

AWorld_DynamicWaterSystem::AWorld_DynamicWaterSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for water updates

    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create water mesh component
    WaterMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMeshComponent"));
    WaterMeshComponent->SetupAttachment(RootComponent);
    WaterMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    WaterMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    WaterMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    WaterMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create water volume component
    WaterVolumeComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("WaterVolumeComponent"));
    WaterVolumeComponent->SetupAttachment(RootComponent);
    WaterVolumeComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    WaterVolumeComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    WaterVolumeComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    WaterVolumeComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create audio component
    WaterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WaterAudioComponent"));
    WaterAudioComponent->SetupAttachment(RootComponent);
    WaterAudioComponent->bAutoActivate = true;

    // Set default values
    WaterType = EWorld_WaterBodyType::Lake;
    WaterDepth = 200.0f;
    WaterTemperature = 15.0f;
    bIsSwimmable = true;
    bHasFish = true;
    
    // Flow data defaults
    FlowData.FlowDirection = FVector::ForwardVector;
    FlowData.FlowSpeed = 100.0f;
    FlowData.FlowIntensity = 1.0f;
    FlowData.bHasCurrents = true;
    FlowData.WaveAmplitude = 5.0f;
    FlowData.WaveFrequency = 0.5f;

    // Audio data defaults
    AudioData.AudioRange = 1000.0f;
    AudioData.VolumeMultiplier = 1.0f;
    AudioData.bUse3DAudio = true;

    // Visual defaults
    WaterColor = FLinearColor(0.0f, 0.3f, 0.8f, 0.7f);
    Transparency = 0.7f;
    ReflectionIntensity = 0.8f;

    // Performance defaults
    LODLevel = 0;
    CullDistance = 5000.0f;
    bEnablePhysicsSimulation = false;
}

void AWorld_DynamicWaterSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize water system
    InitializeWaterBody(WaterType, GetActorScale3D());
    
    // Set up collision events
    if (WaterVolumeComponent)
    {
        WaterVolumeComponent->OnComponentBeginOverlap.AddDynamic(this, &AWorld_DynamicWaterSystem::OnWaterVolumeBeginOverlap);
        WaterVolumeComponent->OnComponentEndOverlap.AddDynamic(this, &AWorld_DynamicWaterSystem::OnWaterVolumeEndOverlap);
    }

    // Update audio based on water type
    UpdateAudioBasedOnFlow();

    UE_LOG(LogTemp, Log, TEXT("DynamicWaterSystem initialized: Type=%d, Depth=%.1f"), 
           static_cast<int32>(WaterType), WaterDepth);
}

void AWorld_DynamicWaterSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CurrentTime += DeltaTime;
    
    // Update water animation
    UpdateWaterAnimation(DeltaTime);
    
    // Check for actors in water
    CheckForActorsInWater();
}

void AWorld_DynamicWaterSystem::InitializeWaterBody(EWorld_WaterBodyType InWaterType, const FVector& InScale)
{
    WaterType = InWaterType;

    if (!WaterMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("WaterMeshComponent is null in InitializeWaterBody"));
        return;
    }

    // Load default plane mesh
    UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
    if (PlaneMesh)
    {
        WaterMeshComponent->SetStaticMesh(PlaneMesh);
        WaterMeshComponent->SetWorldScale3D(InScale);
    }

    // Set water volume size
    if (WaterVolumeComponent)
    {
        FVector BoxExtent = FVector(InScale.X * 50.0f, InScale.Y * 50.0f, WaterDepth * 0.5f);
        WaterVolumeComponent->SetBoxExtent(BoxExtent);
        WaterVolumeComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -WaterDepth * 0.5f));
    }

    // Configure based on water type
    switch (WaterType)
    {
        case EWorld_WaterBodyType::River:
            FlowData.FlowSpeed = 150.0f;
            FlowData.FlowIntensity = 1.5f;
            FlowData.bHasCurrents = true;
            WaterTemperature = 12.0f;
            break;

        case EWorld_WaterBodyType::Lake:
            FlowData.FlowSpeed = 20.0f;
            FlowData.FlowIntensity = 0.3f;
            FlowData.bHasCurrents = false;
            WaterTemperature = 18.0f;
            break;

        case EWorld_WaterBodyType::Pond:
            FlowData.FlowSpeed = 5.0f;
            FlowData.FlowIntensity = 0.1f;
            FlowData.bHasCurrents = false;
            WaterTemperature = 20.0f;
            break;

        case EWorld_WaterBodyType::Stream:
            FlowData.FlowSpeed = 100.0f;
            FlowData.FlowIntensity = 1.0f;
            FlowData.bHasCurrents = true;
            WaterTemperature = 10.0f;
            break;

        case EWorld_WaterBodyType::Waterfall:
            FlowData.FlowSpeed = 300.0f;
            FlowData.FlowIntensity = 2.0f;
            FlowData.bHasCurrents = true;
            WaterTemperature = 8.0f;
            break;
    }

    // Update material
    UpdateWaterMaterial();

    UE_LOG(LogTemp, Log, TEXT("Water body initialized: Type=%d, Scale=(%s)"), 
           static_cast<int32>(WaterType), *InScale.ToString());
}

void AWorld_DynamicWaterSystem::SetWaterFlow(const FVector& FlowDirection, float FlowSpeed)
{
    FlowData.FlowDirection = FlowDirection.GetSafeNormal();
    FlowData.FlowSpeed = FlowSpeed;
    
    // Update audio based on new flow
    UpdateAudioBasedOnFlow();

    UE_LOG(LogTemp, Log, TEXT("Water flow updated: Direction=(%s), Speed=%.1f"), 
           *FlowDirection.ToString(), FlowSpeed);
}

void AWorld_DynamicWaterSystem::UpdateWaterMaterial()
{
    if (!WaterMeshComponent)
    {
        return;
    }

    // Create dynamic material instance if we have a base material
    if (WaterMaterial.IsValid())
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(WaterMaterial.Get(), this);
        if (DynamicMaterial)
        {
            // Set material parameters
            DynamicMaterial->SetVectorParameterValue(FName("WaterColor"), WaterColor);
            DynamicMaterial->SetScalarParameterValue(FName("Transparency"), Transparency);
            DynamicMaterial->SetScalarParameterValue(FName("ReflectionIntensity"), ReflectionIntensity);
            DynamicMaterial->SetScalarParameterValue(FName("FlowSpeed"), FlowData.FlowSpeed);
            DynamicMaterial->SetVectorParameterValue(FName("FlowDirection"), FlowData.FlowDirection);

            WaterMeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
}

void AWorld_DynamicWaterSystem::SetWaterAudio(USoundCue* AmbientSound, USoundCue* FlowSound)
{
    if (!WaterAudioComponent)
    {
        return;
    }

    // Set ambient sound based on flow intensity
    USoundCue* SoundToUse = (FlowData.FlowIntensity > 0.5f) ? FlowSound : AmbientSound;
    
    if (SoundToUse)
    {
        WaterAudioComponent->SetSound(SoundToUse);
        WaterAudioComponent->SetVolumeMultiplier(AudioData.VolumeMultiplier);
        
        if (AudioData.bUse3DAudio)
        {
            WaterAudioComponent->bAllowSpatialization = true;
            WaterAudioComponent->AttenuationOverrides.bOverrideAttenuation = true;
            WaterAudioComponent->AttenuationOverrides.FalloffDistance = AudioData.AudioRange;
        }
    }
}

bool AWorld_DynamicWaterSystem::IsPointInWater(const FVector& Point) const
{
    if (!WaterVolumeComponent)
    {
        return false;
    }

    // Check if point is within water volume bounds
    FVector LocalPoint = GetActorTransform().InverseTransformPosition(Point);
    FVector BoxExtent = WaterVolumeComponent->GetScaledBoxExtent();
    FVector BoxCenter = WaterVolumeComponent->GetRelativeLocation();

    return (FMath::Abs(LocalPoint.X - BoxCenter.X) <= BoxExtent.X &&
            FMath::Abs(LocalPoint.Y - BoxCenter.Y) <= BoxExtent.Y &&
            FMath::Abs(LocalPoint.Z - BoxCenter.Z) <= BoxExtent.Z);
}

float AWorld_DynamicWaterSystem::GetWaterDepthAtPoint(const FVector& Point) const
{
    if (!IsPointInWater(Point))
    {
        return 0.0f;
    }

    // Calculate depth based on water surface level
    float WaterSurfaceZ = GetActorLocation().Z;
    float PointZ = Point.Z;
    
    return FMath::Max(0.0f, WaterSurfaceZ - PointZ);
}

FVector AWorld_DynamicWaterSystem::GetWaterFlowAtPoint(const FVector& Point) const
{
    if (!IsPointInWater(Point) || !FlowData.bHasCurrents)
    {
        return FVector::ZeroVector;
    }

    // Apply flow direction and speed
    FVector WorldFlowDirection = GetActorTransform().TransformVectorNoScale(FlowData.FlowDirection);
    return WorldFlowDirection * FlowData.FlowSpeed * FlowData.FlowIntensity;
}

void AWorld_DynamicWaterSystem::SetLODLevel(int32 NewLODLevel)
{
    LODLevel = FMath::Clamp(NewLODLevel, 0, 3);
    
    if (WaterMeshComponent)
    {
        WaterMeshComponent->SetForcedLodModel(LODLevel + 1);
    }
}

void AWorld_DynamicWaterSystem::OptimizeForDistance(float DistanceToPlayer)
{
    if (DistanceToPlayer > CullDistance)
    {
        SetActorHiddenInGame(true);
        SetActorTickEnabled(false);
        return;
    }

    SetActorHiddenInGame(false);
    SetActorTickEnabled(true);

    // Adjust LOD based on distance
    int32 NewLOD = 0;
    if (DistanceToPlayer > 2000.0f)
    {
        NewLOD = 2;
    }
    else if (DistanceToPlayer > 1000.0f)
    {
        NewLOD = 1;
    }

    SetLODLevel(NewLOD);
}

void AWorld_DynamicWaterSystem::UpdateWaterAnimation(float DeltaTime)
{
    if (!WaterMeshComponent)
    {
        return;
    }

    // Update wave animation
    float WaveOffset = FMath::Sin(CurrentTime * FlowData.WaveFrequency) * FlowData.WaveAmplitude;
    FVector CurrentLocation = GetActorLocation();
    CurrentLocation.Z += WaveOffset * 0.1f; // Subtle wave movement
    
    // Only update if wave amplitude is significant
    if (FlowData.WaveAmplitude > 1.0f)
    {
        SetActorLocation(CurrentLocation);
    }
}

void AWorld_DynamicWaterSystem::UpdateAudioBasedOnFlow()
{
    if (!WaterAudioComponent)
    {
        return;
    }

    // Adjust audio volume based on flow intensity
    float VolumeMultiplier = AudioData.VolumeMultiplier * FlowData.FlowIntensity;
    WaterAudioComponent->SetVolumeMultiplier(VolumeMultiplier);

    // Adjust pitch based on flow speed
    float PitchMultiplier = 1.0f + (FlowData.FlowSpeed / 200.0f) * 0.2f;
    WaterAudioComponent->SetPitchMultiplier(FMath::Clamp(PitchMultiplier, 0.8f, 1.5f));
}

void AWorld_DynamicWaterSystem::CheckForActorsInWater()
{
    if (!WaterVolumeComponent)
    {
        return;
    }

    // Get overlapping actors
    TArray<AActor*> OverlappingActors;
    WaterVolumeComponent->GetOverlappingActors(OverlappingActors);

    // Check for new actors entering water
    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && !ActorsInWater.Contains(Actor))
        {
            ActorsInWater.Add(Actor);
            ApplyWaterEffectsToActor(Actor);
            OnActorEnteredWater(Actor);
        }
    }

    // Check for actors leaving water
    for (int32 i = ActorsInWater.Num() - 1; i >= 0; i--)
    {
        AActor* Actor = ActorsInWater[i];
        if (!Actor || !OverlappingActors.Contains(Actor))
        {
            if (Actor)
            {
                RemoveWaterEffectsFromActor(Actor);
                OnActorExitedWater(Actor);
            }
            ActorsInWater.RemoveAt(i);
        }
    }
}

void AWorld_DynamicWaterSystem::ApplyWaterEffectsToActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    // Apply water flow force if actor has physics
    UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
    if (PrimComp && PrimComp->IsSimulatingPhysics() && FlowData.bHasCurrents)
    {
        FVector FlowForce = GetWaterFlowAtPoint(Actor->GetActorLocation());
        PrimComp->AddForce(FlowForce * PrimComp->GetMass());
    }

    UE_LOG(LogTemp, Log, TEXT("Applied water effects to actor: %s"), *Actor->GetName());
}

void AWorld_DynamicWaterSystem::RemoveWaterEffectsFromActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Removed water effects from actor: %s"), *Actor->GetName());
}

// Collision event handlers
UFUNCTION()
void AWorld_DynamicWaterSystem::OnWaterVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && !ActorsInWater.Contains(OtherActor))
    {
        ActorsInWater.Add(OtherActor);
        ApplyWaterEffectsToActor(OtherActor);
        OnActorEnteredWater(OtherActor);
    }
}

UFUNCTION()
void AWorld_DynamicWaterSystem::OnWaterVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (OtherActor && ActorsInWater.Contains(OtherActor))
    {
        ActorsInWater.Remove(OtherActor);
        RemoveWaterEffectsFromActor(OtherActor);
        OnActorExitedWater(OtherActor);
    }
}