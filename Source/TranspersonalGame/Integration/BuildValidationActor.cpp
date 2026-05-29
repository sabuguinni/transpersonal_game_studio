#include "BuildValidationActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

ABuildValidationActor::ABuildValidationActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create validation mesh component
    ValidationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ValidationMesh"));
    ValidationMesh->SetupAttachment(RootComponent);
    
    // Set default mesh to a simple cube
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        ValidationMesh->SetStaticMesh(CubeMeshAsset.Object);
    }
    
    // Initialize validation state
    bIsValidationPassing = false;
    ValidationScore = 0.0f;
    LastValidationTime = 0.0f;
    ValidationInterval = 5.0f;
    
    // Set default scale
    SetActorScale3D(FVector(0.5f, 0.5f, 0.5f));
}

void ABuildValidationActor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationActor: BeginPlay - Starting validation monitoring"));
    
    // Perform initial validation
    PerformValidation();
}

void ABuildValidationActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastValidationTime += DeltaTime;
    
    // Perform validation at intervals
    if (LastValidationTime >= ValidationInterval)
    {
        PerformValidation();
        LastValidationTime = 0.0f;
    }
    
    // Update visual feedback
    UpdateVisualFeedback();
}

void ABuildValidationActor::PerformValidation()
{
    UE_LOG(LogTemp, Log, TEXT("BuildValidationActor: Performing validation check"));
    
    float NewScore = 0.0f;
    bool bAllTestsPassed = true;
    
    // Test 1: Check if world is valid
    if (GetWorld())
    {
        NewScore += 25.0f;
        UE_LOG(LogTemp, Log, TEXT("BuildValidationActor: World validation PASSED"));
    }
    else
    {
        bAllTestsPassed = false;
        UE_LOG(LogTemp, Error, TEXT("BuildValidationActor: World validation FAILED"));
    }
    
    // Test 2: Check if actor is properly placed
    FVector ActorLocation = GetActorLocation();
    if (!ActorLocation.IsZero())
    {
        NewScore += 25.0f;
        UE_LOG(LogTemp, Log, TEXT("BuildValidationActor: Location validation PASSED (%.2f, %.2f, %.2f)"), 
               ActorLocation.X, ActorLocation.Y, ActorLocation.Z);
    }
    else
    {
        bAllTestsPassed = false;
        UE_LOG(LogTemp, Warning, TEXT("BuildValidationActor: Location validation WARNING - Actor at origin"));
    }
    
    // Test 3: Check if mesh component is valid
    if (ValidationMesh && ValidationMesh->GetStaticMesh())
    {
        NewScore += 25.0f;
        UE_LOG(LogTemp, Log, TEXT("BuildValidationActor: Mesh validation PASSED"));
    }
    else
    {
        bAllTestsPassed = false;
        UE_LOG(LogTemp, Error, TEXT("BuildValidationActor: Mesh validation FAILED"));
    }
    
    // Test 4: Check if tick is working
    if (LastValidationTime >= 0.0f)
    {
        NewScore += 25.0f;
        UE_LOG(LogTemp, Log, TEXT("BuildValidationActor: Tick validation PASSED"));
    }
    else
    {
        bAllTestsPassed = false;
        UE_LOG(LogTemp, Error, TEXT("BuildValidationActor: Tick validation FAILED"));
    }
    
    // Update validation state
    ValidationScore = NewScore;
    bIsValidationPassing = bAllTestsPassed && (ValidationScore >= 75.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationActor: Validation complete - Score: %.1f%%, Status: %s"), 
           ValidationScore, bIsValidationPassing ? TEXT("PASS") : TEXT("FAIL"));
    
    // Broadcast validation result
    OnValidationComplete.Broadcast(bIsValidationPassing, ValidationScore);
}

void ABuildValidationActor::UpdateVisualFeedback()
{
    if (!ValidationMesh)
        return;
    
    // Change color based on validation status
    if (bIsValidationPassing)
    {
        // Green for passing validation
        ValidationMesh->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(0.0f, 1.0f, 0.0f));
    }
    else if (ValidationScore > 50.0f)
    {
        // Yellow for partial validation
        ValidationMesh->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(1.0f, 1.0f, 0.0f));
    }
    else
    {
        // Red for failing validation
        ValidationMesh->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), FVector(1.0f, 0.0f, 0.0f));
    }
    
    // Scale based on score (0.2 to 1.0)
    float ScaleFactor = 0.2f + (ValidationScore / 100.0f) * 0.8f;
    SetActorScale3D(FVector(ScaleFactor, ScaleFactor, ScaleFactor));
}

void ABuildValidationActor::ForceValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationActor: Force validation requested"));
    PerformValidation();
}

void ABuildValidationActor::SetValidationInterval(float NewInterval)
{
    ValidationInterval = FMath::Max(1.0f, NewInterval);
    UE_LOG(LogTemp, Log, TEXT("BuildValidationActor: Validation interval set to %.1f seconds"), ValidationInterval);
}

float ABuildValidationActor::GetValidationScore() const
{
    return ValidationScore;
}

bool ABuildValidationActor::IsValidationPassing() const
{
    return bIsValidationPassing;
}