#include "World_RiverSystemGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

UWorld_RiverSystemGenerator::UWorld_RiverSystemGenerator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    // Initialize generation parameters
    MaxRiverSegments = 20;
    MinRiverWidth = 300.0f;
    MaxRiverWidth = 800.0f;
    RiverMeandering = 0.3f;
    TributaryCount = 3;
    LakeProbability = 0.15f;

    // Initialize materials to nullptr - will be set in Blueprint or at runtime
    RiverWaterMaterial = nullptr;
    LakeWaterMaterial = nullptr;
    RiverbankMaterial = nullptr;
}

void UWorld_RiverSystemGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_RiverSystemGenerator: Component initialized"));
}

void UWorld_RiverSystemGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWorld_RiverSystemGenerator::GenerateRiverNetwork(FVector StartLocation, FVector EndLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("Generating river network from %s to %s"), *StartLocation.ToString(), *EndLocation.ToString());

    // Clear existing network
    GeneratedNetwork = FWorld_RiverNetwork();
    ClearGeneratedRivers();

    // Set source and delta locations
    GeneratedNetwork.SourceLocation = StartLocation;
    GeneratedNetwork.DeltaLocation = EndLocation;

    // Calculate total distance
    float TotalDistance = FVector::Dist(StartLocation, EndLocation);
    GeneratedNetwork.TotalLength = TotalDistance;

    // Generate main river path with meandering
    FVector CurrentLocation = StartLocation;
    FVector TargetLocation = EndLocation;
    
    for (int32 i = 0; i < MaxRiverSegments && FVector::Dist(CurrentLocation, TargetLocation) > 1000.0f; i++)
    {
        // Calculate segment length
        float SegmentLength = FMath::Min(TotalDistance / MaxRiverSegments, FVector::Dist(CurrentLocation, TargetLocation));
        
        // Calculate next point with meandering
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        FVector MeanderOffset = FVector(
            FMath::RandRange(-RiverMeandering, RiverMeandering) * SegmentLength,
            FMath::RandRange(-RiverMeandering, RiverMeandering) * SegmentLength,
            0.0f
        );
        
        FVector NextLocation = CurrentLocation + Direction * SegmentLength + MeanderOffset;
        
        // Adjust for terrain elevation
        NextLocation.Z = CalculateTerrainElevation(NextLocation);
        
        // Create river segment
        float SegmentWidth = FMath::RandRange(MinRiverWidth, MaxRiverWidth);
        FWorld_RiverSegment NewSegment = CreateRiverSegment(CurrentLocation, NextLocation, SegmentWidth, true);
        
        GeneratedNetwork.MainRiver.Add(NewSegment);
        
        // Generate tributaries for this segment
        if (FMath::RandRange(0.0f, 1.0f) < 0.4f) // 40% chance for tributary
        {
            GenerateTributaries(NewSegment);
        }
        
        // Generate lake
        if (FMath::RandRange(0.0f, 1.0f) < LakeProbability)
        {
            FVector LakeLocation = FMath::Lerp(CurrentLocation, NextLocation, 0.5f);
            GeneratedNetwork.LakePositions.Add(LakeLocation);
        }
        
        CurrentLocation = NextLocation;
    }

    // Final segment to target
    if (FVector::Dist(CurrentLocation, TargetLocation) > 100.0f)
    {
        float FinalWidth = FMath::RandRange(MinRiverWidth, MaxRiverWidth);
        FWorld_RiverSegment FinalSegment = CreateRiverSegment(CurrentLocation, TargetLocation, FinalWidth, true);
        GeneratedNetwork.MainRiver.Add(FinalSegment);
    }

    UE_LOG(LogTemp, Warning, TEXT("Generated river network with %d main segments, %d tributaries, %d lakes"), 
           GeneratedNetwork.MainRiver.Num(), GeneratedNetwork.Tributaries.Num(), GeneratedNetwork.LakePositions.Num());
}

void UWorld_RiverSystemGenerator::GenerateTributaries(const FWorld_RiverSegment& MainSegment)
{
    int32 NumTributaries = FMath::RandRange(1, TributaryCount);
    
    for (int32 i = 0; i < NumTributaries; i++)
    {
        // Random point along main segment
        float Alpha = FMath::RandRange(0.2f, 0.8f);
        FVector JunctionPoint = FMath::Lerp(MainSegment.StartPoint, MainSegment.EndPoint, Alpha);
        
        // Generate tributary direction (perpendicular to main river)
        FVector MainDirection = (MainSegment.EndPoint - MainSegment.StartPoint).GetSafeNormal();
        FVector TributaryDirection = FVector::CrossProduct(MainDirection, FVector::UpVector);
        
        // Randomize direction
        if (FMath::RandBool())
        {
            TributaryDirection *= -1.0f;
        }
        
        // Generate tributary length
        float TributaryLength = FMath::RandRange(1000.0f, 3000.0f);
        FVector TributaryEnd = JunctionPoint + TributaryDirection * TributaryLength;
        TributaryEnd.Z = CalculateTerrainElevation(TributaryEnd) + FMath::RandRange(50.0f, 200.0f); // Higher elevation
        
        // Create tributary segment
        float TributaryWidth = MainSegment.Width * FMath::RandRange(0.3f, 0.7f);
        FWorld_RiverSegment Tributary = CreateRiverSegment(TributaryEnd, JunctionPoint, TributaryWidth, false);
        
        GeneratedNetwork.Tributaries.Add(Tributary);
    }
}

void UWorld_RiverSystemGenerator::SpawnRiverMeshes()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn river meshes - no world context"));
        return;
    }

    // Spawn main river segments
    for (const FWorld_RiverSegment& Segment : GeneratedNetwork.MainRiver)
    {
        AActor* RiverActor = SpawnRiverSegmentMesh(Segment);
        if (RiverActor)
        {
            SpawnedRiverActors.Add(RiverActor);
        }
    }

    // Spawn tributary segments
    for (const FWorld_RiverSegment& Tributary : GeneratedNetwork.Tributaries)
    {
        AActor* TributaryActor = SpawnRiverSegmentMesh(Tributary);
        if (TributaryActor)
        {
            SpawnedRiverActors.Add(TributaryActor);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Spawned %d river mesh actors"), SpawnedRiverActors.Num());
}

void UWorld_RiverSystemGenerator::SpawnLakes()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (const FVector& LakePosition : GeneratedNetwork.LakePositions)
    {
        float LakeRadius = FMath::RandRange(500.0f, 1500.0f);
        AActor* LakeActor = SpawnLakeAtLocation(LakePosition, LakeRadius);
        if (LakeActor)
        {
            SpawnedRiverActors.Add(LakeActor);
        }
    }
}

void UWorld_RiverSystemGenerator::SpawnRiverbanks()
{
    // Generate riverbank geometry for all river segments
    for (const FWorld_RiverSegment& Segment : GeneratedNetwork.MainRiver)
    {
        GenerateRiverbankGeometry(Segment);
    }

    for (const FWorld_RiverSegment& Tributary : GeneratedNetwork.Tributaries)
    {
        GenerateRiverbankGeometry(Tributary);
    }
}

void UWorld_RiverSystemGenerator::ClearGeneratedRivers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (AActor* Actor : SpawnedRiverActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    SpawnedRiverActors.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Cleared all generated river actors"));
}

FVector UWorld_RiverSystemGenerator::CalculateRiverFlow(const FVector& CurrentPoint, const FVector& TargetPoint)
{
    FVector FlowDirection = (TargetPoint - CurrentPoint).GetSafeNormal();
    
    // Add downhill bias
    FVector DownhillDirection = GetDownhillDirection(CurrentPoint);
    FlowDirection = (FlowDirection + DownhillDirection * 0.3f).GetSafeNormal();
    
    return FlowDirection;
}

bool UWorld_RiverSystemGenerator::IsValidRiverLocation(const FVector& Location)
{
    // Check if location is suitable for river placement
    // For now, always return true - can be expanded with terrain analysis
    return true;
}

void UWorld_RiverSystemGenerator::GenerateTestRiver()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating test river network"));
    
    FVector StartLoc = GetOwner()->GetActorLocation() + FVector(0, 0, 500);
    FVector EndLoc = StartLoc + FVector(5000, 3000, -200);
    
    GenerateRiverNetwork(StartLoc, EndLoc);
    SpawnRiverMeshes();
    SpawnLakes();
    SpawnRiverbanks();
}

// Private implementation functions

FWorld_RiverSegment UWorld_RiverSystemGenerator::CreateRiverSegment(const FVector& Start, const FVector& End, float Width, bool bMainChannel)
{
    FWorld_RiverSegment Segment;
    Segment.StartPoint = Start;
    Segment.EndPoint = End;
    Segment.Width = Width;
    Segment.Depth = Width * 0.1f; // Depth proportional to width
    Segment.FlowSpeed = bMainChannel ? 300.0f : 150.0f;
    Segment.bIsMainChannel = bMainChannel;
    
    return Segment;
}

void UWorld_RiverSystemGenerator::ApplyRiverMeandering(FWorld_RiverSegment& Segment)
{
    // Apply natural river meandering patterns
    FVector Direction = (Segment.EndPoint - Segment.StartPoint);
    float SegmentLength = Direction.Size();
    Direction.Normalize();
    
    // Create meandering offset
    FVector Perpendicular = FVector::CrossProduct(Direction, FVector::UpVector);
    float MeanderAmount = SegmentLength * RiverMeandering * FMath::RandRange(-1.0f, 1.0f);
    
    FVector MidPoint = (Segment.StartPoint + Segment.EndPoint) * 0.5f;
    MidPoint += Perpendicular * MeanderAmount;
    
    // Update segment to include meandering
    // For simplicity, we'll adjust the endpoint slightly
    Segment.EndPoint += Perpendicular * MeanderAmount * 0.3f;
}

AActor* UWorld_RiverSystemGenerator::SpawnRiverSegmentMesh(const FWorld_RiverSegment& Segment)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Create a static mesh actor for the river segment
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AStaticMeshActor* RiverActor = World->SpawnActor<AStaticMeshActor>(SpawnParams);
    if (!RiverActor)
    {
        return nullptr;
    }

    // Position the actor at the segment center
    FVector SegmentCenter = (Segment.StartPoint + Segment.EndPoint) * 0.5f;
    RiverActor->SetActorLocation(SegmentCenter);
    
    // Calculate rotation to align with river flow
    FVector FlowDirection = (Segment.EndPoint - Segment.StartPoint).GetSafeNormal();
    FRotator FlowRotation = FlowDirection.Rotation();
    RiverActor->SetActorRotation(FlowRotation);
    
    // Scale based on segment dimensions
    float SegmentLength = FVector::Dist(Segment.StartPoint, Segment.EndPoint);
    FVector Scale = FVector(SegmentLength / 100.0f, Segment.Width / 100.0f, 1.0f);
    RiverActor->SetActorScale3D(Scale);
    
    // Set material if available
    if (RiverWaterMaterial && RiverActor->GetStaticMeshComponent())
    {
        RiverActor->GetStaticMeshComponent()->SetMaterial(0, RiverWaterMaterial);
    }
    
    // Set actor label for identification
    FString ActorLabel = Segment.bIsMainChannel ? TEXT("MainRiver_Segment") : TEXT("Tributary_Segment");
    RiverActor->SetActorLabel(ActorLabel);
    
    return RiverActor;
}

AActor* UWorld_RiverSystemGenerator::SpawnLakeAtLocation(const FVector& Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AStaticMeshActor* LakeActor = World->SpawnActor<AStaticMeshActor>(SpawnParams);
    if (!LakeActor)
    {
        return nullptr;
    }

    LakeActor->SetActorLocation(Location);
    
    // Scale based on lake radius
    FVector LakeScale = FVector(Radius / 100.0f, Radius / 100.0f, 1.0f);
    LakeActor->SetActorScale3D(LakeScale);
    
    // Set lake material if available
    if (LakeWaterMaterial && LakeActor->GetStaticMeshComponent())
    {
        LakeActor->GetStaticMeshComponent()->SetMaterial(0, LakeWaterMaterial);
    }
    
    LakeActor->SetActorLabel(TEXT("PrehistoricLake"));
    
    return LakeActor;
}

void UWorld_RiverSystemGenerator::GenerateRiverbankGeometry(const FWorld_RiverSegment& Segment)
{
    // Generate riverbank geometry along the segment
    // This is a simplified implementation - could be expanded with more detailed mesh generation
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FVector Direction = (Segment.EndPoint - Segment.StartPoint).GetSafeNormal();
    FVector Perpendicular = FVector::CrossProduct(Direction, FVector::UpVector);
    
    // Create riverbank actors on both sides
    for (int32 Side = -1; Side <= 1; Side += 2)
    {
        FVector BankOffset = Perpendicular * Side * (Segment.Width * 0.6f);
        FVector BankStart = Segment.StartPoint + BankOffset;
        FVector BankEnd = Segment.EndPoint + BankOffset;
        FVector BankCenter = (BankStart + BankEnd) * 0.5f;
        
        FActorSpawnParameters SpawnParams;
        AStaticMeshActor* BankActor = World->SpawnActor<AStaticMeshActor>(SpawnParams);
        if (BankActor)
        {
            BankActor->SetActorLocation(BankCenter);
            BankActor->SetActorLabel(TEXT("Riverbank"));
            
            if (RiverbankMaterial && BankActor->GetStaticMeshComponent())
            {
                BankActor->GetStaticMeshComponent()->SetMaterial(0, RiverbankMaterial);
            }
            
            SpawnedRiverActors.Add(BankActor);
        }
    }
}

float UWorld_RiverSystemGenerator::CalculateTerrainElevation(const FVector& Location)
{
    // Simple terrain elevation calculation
    // In a real implementation, this would query the actual terrain heightmap
    
    // Create some basic elevation variation
    float BaseElevation = 0.0f;
    float NoiseValue = FMath::PerlinNoise2D(FVector2D(Location.X / 1000.0f, Location.Y / 1000.0f));
    
    return BaseElevation + NoiseValue * 200.0f;
}

FVector UWorld_RiverSystemGenerator::GetDownhillDirection(const FVector& Location)
{
    // Calculate downhill direction based on terrain gradient
    // Simplified implementation - sample nearby points and find steepest descent
    
    float SampleDistance = 100.0f;
    TArray<FVector> SamplePoints = {
        Location + FVector(SampleDistance, 0, 0),
        Location + FVector(-SampleDistance, 0, 0),
        Location + FVector(0, SampleDistance, 0),
        Location + FVector(0, -SampleDistance, 0)
    };
    
    FVector DownhillDirection = FVector::ZeroVector;
    float CurrentElevation = CalculateTerrainElevation(Location);
    float MaxGradient = 0.0f;
    
    for (const FVector& SamplePoint : SamplePoints)
    {
        float SampleElevation = CalculateTerrainElevation(SamplePoint);
        float Gradient = (CurrentElevation - SampleElevation) / SampleDistance;
        
        if (Gradient > MaxGradient)
        {
            MaxGradient = Gradient;
            DownhillDirection = (SamplePoint - Location).GetSafeNormal();
        }
    }
    
    return DownhillDirection;
}