#include "Char_TribalNPC.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AChar_TribalNPC::AChar_TribalNPC()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize tribal properties
    TribalRole = EChar_TribalRole::Gatherer;
    NPCName = TEXT("Tribal Member");
    Age = 25;
    WalkSpeed = 150.0f;
    InteractionRadius = 200.0f;
    Health = 100.0f;
    Hunger = 50.0f;
    Fatigue = 30.0f;

    // Initialize appearance with default values
    Appearance.SkinTone = 0.6f;
    Appearance.HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
    Appearance.EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
    Appearance.BodyWeight = 0.5f;
    Appearance.Height = 1.0f;

    // Create tool mesh component
    ToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMesh"));
    if (ToolMesh)
    {
        ToolMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
        ToolMesh->SetRelativeLocation(FVector(10.0f, 0.0f, 0.0f));
        ToolMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.8f));
    }

    // Create clothing mesh component
    ClothingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClothingMesh"));
    if (ClothingMesh)
    {
        ClothingMesh->SetupAttachment(GetMesh());
        ClothingMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    }

    // Configure character movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    }

    // Set collision
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);
}

void AChar_TribalNPC::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTribalNPC();
    SetupRoleSpecificEquipment();
    ApplyAppearanceSettings();
}

void AChar_TribalNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update survival stats over time
    Hunger = FMath::Clamp(Hunger + (DeltaTime * 0.1f), 0.0f, 100.0f);
    Fatigue = FMath::Clamp(Fatigue + (DeltaTime * 0.05f), 0.0f, 100.0f);
}

void AChar_TribalNPC::SetTribalRole(EChar_TribalRole NewRole)
{
    TribalRole = NewRole;
    SetupRoleSpecificEquipment();
    
    // Update movement speed based on role
    switch (TribalRole)
    {
        case EChar_TribalRole::Hunter:
            WalkSpeed = 180.0f;
            break;
        case EChar_TribalRole::Warrior:
            WalkSpeed = 160.0f;
            break;
        case EChar_TribalRole::Gatherer:
            WalkSpeed = 140.0f;
            break;
        case EChar_TribalRole::Crafter:
            WalkSpeed = 120.0f;
            break;
        case EChar_TribalRole::Elder:
            WalkSpeed = 100.0f;
            break;
    }
    
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void AChar_TribalNPC::UpdateAppearance(const FChar_TribalAppearance& NewAppearance)
{
    Appearance = NewAppearance;
    ApplyAppearanceSettings();
}

void AChar_TribalNPC::EquipTool(UStaticMesh* NewToolMesh)
{
    if (ToolMesh && NewToolMesh)
    {
        ToolMesh->SetStaticMesh(NewToolMesh);
        ToolMesh->SetVisibility(true);
    }
}

FString AChar_TribalNPC::GetRoleDescription() const
{
    switch (TribalRole)
    {
        case EChar_TribalRole::Hunter:
            return TEXT("Skilled in tracking and hunting prey");
        case EChar_TribalRole::Gatherer:
            return TEXT("Expert at finding plants and resources");
        case EChar_TribalRole::Crafter:
            return TEXT("Creates tools and equipment for the tribe");
        case EChar_TribalRole::Elder:
            return TEXT("Wise leader with knowledge of the old ways");
        case EChar_TribalRole::Warrior:
            return TEXT("Protector of the tribe against dangers");
        default:
            return TEXT("Member of the tribe");
    }
}

bool AChar_TribalNPC::CanInteractWithPlayer() const
{
    return Health > 0.0f && Fatigue < 90.0f;
}

void AChar_TribalNPC::InitializeTribalNPC()
{
    // Generate random variations for this NPC
    Age = FMath::RandRange(18, 65);
    
    // Adjust appearance based on age
    if (Age > 50)
    {
        Appearance.HairColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f); // Gray hair
        WalkSpeed *= 0.8f; // Slower movement
    }
    else if (Age < 25)
    {
        Appearance.BodyWeight = FMath::RandRange(0.3f, 0.6f); // Younger, leaner
    }
    
    // Random appearance variations
    Appearance.SkinTone = FMath::RandRange(0.3f, 0.8f);
    Appearance.Height = FMath::RandRange(0.9f, 1.1f);
    
    // Apply height to mesh scale
    if (GetMesh())
    {
        FVector CurrentScale = GetMesh()->GetRelativeScale3D();
        GetMesh()->SetRelativeScale3D(FVector(CurrentScale.X, CurrentScale.Y, CurrentScale.Z * Appearance.Height));
    }
}

void AChar_TribalNPC::SetupRoleSpecificEquipment()
{
    if (!ToolMesh)
        return;
        
    // Hide tool by default
    ToolMesh->SetVisibility(false);
    
    // Role-specific equipment setup
    switch (TribalRole)
    {
        case EChar_TribalRole::Hunter:
            // Will attach spear or bow when available
            ToolMesh->SetRelativeLocation(FVector(15.0f, 0.0f, 0.0f));
            break;
            
        case EChar_TribalRole::Gatherer:
            // Will attach basket or gathering tool
            ToolMesh->SetRelativeLocation(FVector(5.0f, -10.0f, 0.0f));
            break;
            
        case EChar_TribalRole::Crafter:
            // Will attach hammer or crafting tool
            ToolMesh->SetRelativeLocation(FVector(8.0f, 0.0f, 0.0f));
            break;
            
        case EChar_TribalRole::Warrior:
            // Will attach weapon
            ToolMesh->SetRelativeLocation(FVector(12.0f, 0.0f, 0.0f));
            break;
            
        case EChar_TribalRole::Elder:
            // Will attach staff or ceremonial item
            ToolMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
            break;
    }
}

void AChar_TribalNPC::ApplyAppearanceSettings()
{
    // Apply appearance settings to mesh materials when available
    if (GetMesh())
    {
        // Scale mesh based on height
        FVector BaseScale = FVector(1.0f, 1.0f, 1.0f);
        GetMesh()->SetRelativeScale3D(BaseScale * Appearance.Height);
        
        // Apply body weight to mesh morph targets when available
        // This would be implemented with actual MetaHuman integration
    }
    
    // Update capsule component size based on height
    if (GetCapsuleComponent())
    {
        float BaseHeight = 88.0f;
        float BaseRadius = 34.0f;
        GetCapsuleComponent()->SetCapsuleHalfHeight(BaseHeight * Appearance.Height);
        GetCapsuleComponent()->SetCapsuleRadius(BaseRadius * (0.8f + (Appearance.BodyWeight * 0.4f)));
    }
}