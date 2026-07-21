#include "Core_MaterialPhysics.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

UCore_MaterialPhysics::UCore_MaterialPhysics()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Set default material type
    MaterialType = ECore_MaterialType::Rock;
    
    // Initialize default properties
    MaterialProperties = GetMaterialPropertiesByType(MaterialType);
}

void UCore_MaterialPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply material properties when the component starts
    ApplyMaterialProperties();
}

void UCore_MaterialPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCore_MaterialPhysics::ApplyMaterialProperties()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Update physical material
    UpdatePhysicalMaterial();

    // Apply to all primitive components on the owner
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PhysicalMaterial)
        {
            PrimComp->SetPhysMaterialOverride(PhysicalMaterial);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Applied material properties for type: %d"), (int32)MaterialType);
}

FCore_MaterialProperties UCore_MaterialPhysics::GetMaterialPropertiesByType(ECore_MaterialType InMaterialType)
{
    FCore_MaterialProperties Properties;

    switch (InMaterialType)
    {
        case ECore_MaterialType::Rock:
            Properties.Friction = 0.8f;
            Properties.Restitution = 0.2f;
            Properties.Density = 2700.0f; // kg/m³ for granite
            Properties.SoundDampening = 0.1f;
            Properties.FootstepVolume = 1.2f;
            Properties.bCanBeDestroyed = false;
            Properties.Hardness = 8.0f;
            break;

        case ECore_MaterialType::Dirt:
            Properties.Friction = 0.6f;
            Properties.Restitution = 0.1f;
            Properties.Density = 1500.0f;
            Properties.SoundDampening = 0.7f;
            Properties.FootstepVolume = 0.8f;
            Properties.bCanBeDestroyed = true;
            Properties.Hardness = 1.0f;
            break;

        case ECore_MaterialType::Sand:
            Properties.Friction = 0.4f;
            Properties.Restitution = 0.05f;
            Properties.Density = 1600.0f;
            Properties.SoundDampening = 0.8f;
            Properties.FootstepVolume = 0.6f;
            Properties.bCanBeDestroyed = true;
            Properties.Hardness = 0.5f;
            break;

        case ECore_MaterialType::Grass:
            Properties.Friction = 0.5f;
            Properties.Restitution = 0.3f;
            Properties.Density = 800.0f;
            Properties.SoundDampening = 0.9f;
            Properties.FootstepVolume = 0.4f;
            Properties.bCanBeDestroyed = true;
            Properties.Hardness = 0.2f;
            break;

        case ECore_MaterialType::Wood:
            Properties.Friction = 0.7f;
            Properties.Restitution = 0.4f;
            Properties.Density = 600.0f;
            Properties.SoundDampening = 0.6f;
            Properties.FootstepVolume = 1.0f;
            Properties.bCanBeDestroyed = true;
            Properties.Hardness = 3.0f;
            break;

        case ECore_MaterialType::Water:
            Properties.Friction = 0.1f;
            Properties.Restitution = 0.0f;
            Properties.Density = 1000.0f;
            Properties.SoundDampening = 1.0f;
            Properties.FootstepVolume = 0.3f;
            Properties.bCanBeDestroyed = false;
            Properties.Hardness = 0.1f;
            break;

        case ECore_MaterialType::Mud:
            Properties.Friction = 0.3f;
            Properties.Restitution = 0.05f;
            Properties.Density = 1800.0f;
            Properties.SoundDampening = 0.9f;
            Properties.FootstepVolume = 0.7f;
            Properties.bCanBeDestroyed = true;
            Properties.Hardness = 0.3f;
            break;

        case ECore_MaterialType::Stone:
            Properties.Friction = 0.9f;
            Properties.Restitution = 0.3f;
            Properties.Density = 2500.0f;
            Properties.SoundDampening = 0.2f;
            Properties.FootstepVolume = 1.3f;
            Properties.bCanBeDestroyed = true;
            Properties.Hardness = 6.0f;
            break;

        case ECore_MaterialType::Bone:
            Properties.Friction = 0.6f;
            Properties.Restitution = 0.2f;
            Properties.Density = 1900.0f;
            Properties.SoundDampening = 0.4f;
            Properties.FootstepVolume = 1.1f;
            Properties.bCanBeDestroyed = true;
            Properties.Hardness = 4.0f;
            break;

        case ECore_MaterialType::Flesh:
            Properties.Friction = 0.8f;
            Properties.Restitution = 0.1f;
            Properties.Density = 1050.0f;
            Properties.SoundDampening = 0.8f;
            Properties.FootstepVolume = 0.5f;
            Properties.bCanBeDestroyed = true;
            Properties.Hardness = 0.5f;
            break;

        default:
            // Default to rock properties
            Properties.Friction = 0.7f;
            Properties.Restitution = 0.3f;
            Properties.Density = 1000.0f;
            Properties.SoundDampening = 0.5f;
            Properties.FootstepVolume = 1.0f;
            Properties.bCanBeDestroyed = true;
            Properties.Hardness = 1.0f;
            break;
    }

    return Properties;
}

void UCore_MaterialPhysics::SetMaterialType(ECore_MaterialType NewMaterialType)
{
    if (MaterialType != NewMaterialType)
    {
        MaterialType = NewMaterialType;
        MaterialProperties = GetMaterialPropertiesByType(MaterialType);
        ApplyMaterialProperties();
    }
}

float UCore_MaterialPhysics::CalculateImpactForce(const FVector& Velocity, float Mass) const
{
    // F = ma, where a is derived from velocity change
    float Speed = Velocity.Size();
    float ImpactForce = Mass * Speed * MaterialProperties.Density / 1000.0f;
    
    return ImpactForce;
}

void UCore_MaterialPhysics::HandleMaterialCollision(UCore_MaterialPhysics* OtherMaterial, const FVector& ImpactPoint, float ImpactForce)
{
    if (!OtherMaterial)
    {
        return;
    }

    // Calculate combined friction and restitution
    float CombinedFriction = (MaterialProperties.Friction + OtherMaterial->MaterialProperties.Friction) * 0.5f;
    float CombinedRestitution = (MaterialProperties.Restitution + OtherMaterial->MaterialProperties.Restitution) * 0.5f;

    // Log collision for debugging
    UE_LOG(LogTemp, Log, TEXT("Material collision: %d vs %d, Force: %f"), 
           (int32)MaterialType, (int32)OtherMaterial->MaterialType, ImpactForce);

    // Apply sound effects based on material types
    float SoundVolume = (MaterialProperties.FootstepVolume + OtherMaterial->MaterialProperties.FootstepVolume) * 0.5f;
    SoundVolume *= FMath::Clamp(ImpactForce / 1000.0f, 0.1f, 2.0f);

    // TODO: Trigger sound effects and particle effects based on materials
    // TODO: Apply damage if impact force exceeds material hardness thresholds
}

void UCore_MaterialPhysics::UpdatePhysicalMaterial()
{
    if (!PhysicalMaterial)
    {
        CreatePhysicalMaterial();
    }

    if (PhysicalMaterial)
    {
        PhysicalMaterial->Friction = MaterialProperties.Friction;
        PhysicalMaterial->Restitution = MaterialProperties.Restitution;
        PhysicalMaterial->Density = MaterialProperties.Density;
    }
}

void UCore_MaterialPhysics::CreatePhysicalMaterial()
{
    // Create a new physical material asset
    PhysicalMaterial = NewObject<UPhysicalMaterial>(this);
    
    if (PhysicalMaterial)
    {
        // Set the name based on material type
        FString MaterialName = FString::Printf(TEXT("PhysMat_%s"), 
                                             *UEnum::GetValueAsString(MaterialType));
        PhysicalMaterial->Rename(*MaterialName);

        // Apply properties
        PhysicalMaterial->Friction = MaterialProperties.Friction;
        PhysicalMaterial->Restitution = MaterialProperties.Restitution;
        PhysicalMaterial->Density = MaterialProperties.Density;

        UE_LOG(LogTemp, Log, TEXT("Created physical material: %s"), *MaterialName);
    }
}