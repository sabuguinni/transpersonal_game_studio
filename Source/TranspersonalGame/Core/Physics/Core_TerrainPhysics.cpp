#include "Core_TerrainPhysics.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Landscape/LandscapeComponent.h"
#include "Landscape/LandscapeInfo.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UCore_TerrainPhysics::UCore_TerrainPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize terrain physics data
    TerrainData = FCore_TerrainPhysicsData();
    LandscapeActor = nullptr;
    TerrainPhysicsMaterial = nullptr;
}

void UCore_TerrainPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTerrainPhysics();
}

void UCore_TerrainPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update terrain physics for owner actor
    if (AActor* Owner = GetOwner())
    {
        CalculateTerrainInteraction(Owner);
    }
}

void UCore_TerrainPhysics::InitializeTerrainPhysics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysics: No world found"));
        return;
    }
    
    // Find landscape actor if not set
    if (!LandscapeActor)
    {
        for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
        {
            LandscapeActor = *ActorItr;
            break;
        }
    }
    
    if (LandscapeActor)
    {
        SetupLandscapeCollision();
        UpdatePhysicsMaterial();
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Initialized with landscape"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysics: No landscape found in world"));
    }
}

void UCore_TerrainPhysics::UpdateTerrainPhysics()
{
    if (LandscapeActor)
    {
        UpdatePhysicsMaterial();
        SetupLandscapeCollision();
    }
}

float UCore_TerrainPhysics::GetTerrainHeightAtLocation(const FVector& WorldLocation)
{
    if (!LandscapeActor)
    {
        return 0.0f;
    }
    
    // Line trace down to find terrain height
    FHitResult HitResult;
    FVector StartLocation = WorldLocation + FVector(0, 0, 10000);
    FVector EndLocation = WorldLocation - FVector(0, 0, 10000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        if (HitResult.GetActor() == LandscapeActor)
        {
            return HitResult.Location.Z;
        }
    }
    
    return WorldLocation.Z;
}

FVector UCore_TerrainPhysics::GetTerrainNormalAtLocation(const FVector& WorldLocation)
{
    if (!LandscapeActor)
    {
        return FVector::UpVector;
    }
    
    // Line trace to get surface normal
    FHitResult HitResult;
    FVector StartLocation = WorldLocation + FVector(0, 0, 1000);
    FVector EndLocation = WorldLocation - FVector(0, 0, 1000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        if (HitResult.GetActor() == LandscapeActor)
        {
            return HitResult.Normal;
        }
    }
    
    return FVector::UpVector;
}

void UCore_TerrainPhysics::ApplyTerrainForces(AActor* Actor, const FVector& Location)
{
    if (!Actor || !LandscapeActor)
    {
        return;
    }
    
    // Get terrain properties at location
    FVector TerrainNormal = GetTerrainNormalAtLocation(Location);
    float TerrainHeight = GetTerrainHeightAtLocation(Location);
    
    // Apply terrain-based forces
    if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
    {
        // Apply friction based on terrain angle
        float SlopeAngle = FMath::Acos(FVector::DotProduct(TerrainNormal, FVector::UpVector));
        float AdjustedFriction = TerrainData.SurfaceFriction * (1.0f - (SlopeAngle / PI));
        
        // Apply downward force on steep slopes
        if (SlopeAngle > FMath::DegreesToRadians(30.0f))
        {
            FVector SlopeForce = -TerrainNormal * 980.0f * PrimComp->GetMass() * FMath::Sin(SlopeAngle);
            PrimComp->AddForce(SlopeForce);
        }
    }
}

bool UCore_TerrainPhysics::IsLocationOnTerrain(const FVector& WorldLocation)
{
    if (!LandscapeActor)
    {
        return false;
    }
    
    float TerrainHeight = GetTerrainHeightAtLocation(WorldLocation);
    float LocationHeight = WorldLocation.Z;
    
    // Check if location is within reasonable distance of terrain
    return FMath::Abs(LocationHeight - TerrainHeight) < 500.0f;
}

void UCore_TerrainPhysics::CalculateTerrainInteraction(AActor* Actor)
{
    if (!Actor || !LandscapeActor)
    {
        return;
    }
    
    FVector ActorLocation = Actor->GetActorLocation();
    
    // Check if actor is on terrain
    if (IsLocationOnTerrain(ActorLocation))
    {
        // Apply terrain forces
        ApplyTerrainForces(Actor, ActorLocation);
        
        // Update actor's ground state
        float TerrainHeight = GetTerrainHeightAtLocation(ActorLocation);
        if (ActorLocation.Z <= TerrainHeight + 100.0f)
        {
            // Actor is on or near ground
            FVector TerrainNormal = GetTerrainNormalAtLocation(ActorLocation);
            
            // Align actor to terrain if needed
            if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
            {
                // Apply terrain alignment for characters
                if (Actor->IsA<APawn>())
                {
                    FRotator CurrentRotation = Actor->GetActorRotation();
                    FRotator TerrainRotation = FRotationMatrix::MakeFromZX(TerrainNormal, Actor->GetActorForwardVector()).Rotator();
                    
                    // Smoothly interpolate to terrain rotation
                    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TerrainRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
                    Actor->SetActorRotation(NewRotation);
                }
            }
        }
    }
}

void UCore_TerrainPhysics::UpdatePhysicsMaterial()
{
    if (!LandscapeActor || !TerrainPhysicsMaterial)
    {
        return;
    }
    
    // Update physics material properties
    TerrainPhysicsMaterial->Friction = TerrainData.SurfaceFriction;
    TerrainPhysicsMaterial->Restitution = TerrainData.Bounciness;
    TerrainPhysicsMaterial->Density = TerrainData.Density;
    
    // Apply to landscape components
    TArray<ULandscapeComponent*> LandscapeComponents;
    LandscapeActor->GetLandscapeInfo()->GetAllLandscapeComponents(LandscapeComponents);
    
    for (ULandscapeComponent* Component : LandscapeComponents)
    {
        if (Component)
        {
            Component->SetPhysMaterialOverride(TerrainPhysicsMaterial);
        }
    }
}

void UCore_TerrainPhysics::SetupLandscapeCollision()
{
    if (!LandscapeActor)
    {
        return;
    }
    
    // Configure landscape collision
    LandscapeActor->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    LandscapeActor->SetCollisionObjectType(ECC_WorldStatic);
    LandscapeActor->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Enable complex collision if needed
    if (TerrainData.bEnableComplexCollision)
    {
        TArray<ULandscapeComponent*> LandscapeComponents;
        LandscapeActor->GetLandscapeInfo()->GetAllLandscapeComponents(LandscapeComponents);
        
        for (ULandscapeComponent* Component : LandscapeComponents)
        {
            if (Component)
            {
                Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Landscape collision configured"));
}