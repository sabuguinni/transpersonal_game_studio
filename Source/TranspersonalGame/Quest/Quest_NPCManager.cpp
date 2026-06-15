#include "Quest_NPCManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

AQuest_NPCActor::AQuest_NPCActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create NPC mesh component
    NPCMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCMesh"));
    NPCMesh->SetupAttachment(RootComponent);

    // Set default mesh to basic shape
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded())
    {
        NPCMesh->SetStaticMesh(CubeMesh.Object);
        NPCMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 2.0f)); // Make it taller like a person
    }

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize NPC data
    CurrentDialogueIndex = 0;
    NPCProfile.NPCName = TEXT("Quest NPC");
    NPCProfile.Role = ENPCRole::QuestGiver;
    NPCProfile.bIsActive = true;
}

void AQuest_NPCActor::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_NPCActor::OnInteractionSphereBeginOverlap);
    }

    UE_LOG(LogTemp, Warning, TEXT("Quest NPC Actor spawned: %s"), *NPCProfile.NPCName);
}

void AQuest_NPCActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AQuest_NPCActor::StartDialogue()
{
    if (NPCProfile.AvailableDialogues.Num() > 0)
    {
        CurrentDialogueIndex = 0;
        FQuest_NPCDialogue CurrentDialogue = GetCurrentDialogue();
        
        UE_LOG(LogTemp, Warning, TEXT("NPC %s says: %s"), *NPCProfile.NPCName, *CurrentDialogue.DialogueText);
        
        // Here we could trigger UI or audio playback
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                FString::Printf(TEXT("%s: %s"), *NPCProfile.NPCName, *CurrentDialogue.DialogueText));
        }
    }
}

FQuest_NPCDialogue AQuest_NPCActor::GetCurrentDialogue()
{
    if (NPCProfile.AvailableDialogues.IsValidIndex(CurrentDialogueIndex))
    {
        return NPCProfile.AvailableDialogues[CurrentDialogueIndex];
    }
    
    // Return empty dialogue if none available
    return FQuest_NPCDialogue();
}

void AQuest_NPCActor::AdvanceDialogue()
{
    if (CurrentDialogueIndex < NPCProfile.AvailableDialogues.Num() - 1)
    {
        CurrentDialogueIndex++;
        FQuest_NPCDialogue NextDialogue = GetCurrentDialogue();
        
        UE_LOG(LogTemp, Warning, TEXT("NPC %s continues: %s"), *NPCProfile.NPCName, *NextDialogue.DialogueText);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, 
                FString::Printf(TEXT("%s: %s"), *NPCProfile.NPCName, *NextDialogue.DialogueText));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC %s has no more dialogue"), *NPCProfile.NPCName);
    }
}

bool AQuest_NPCActor::HasQuestAvailable()
{
    return NPCProfile.QuestIDs.Num() > 0;
}

TArray<FString> AQuest_NPCActor::GetAvailableQuests()
{
    return NPCProfile.QuestIDs;
}

void AQuest_NPCActor::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is the player
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        UE_LOG(LogTemp, Warning, TEXT("Player approached NPC: %s"), *NPCProfile.NPCName);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                FString::Printf(TEXT("Press E to talk to %s"), *NPCProfile.NPCName));
        }
        
        // Auto-start dialogue for demo purposes
        StartDialogue();
    }
}

// UQuest_NPCManager Implementation
void UQuest_NPCManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest NPC Manager initialized"));
    
    // Setup default NPCs
    SetupDefaultNPCs();
}

void UQuest_NPCManager::Deinitialize()
{
    // Clean up active NPC actors
    for (AQuest_NPCActor* NPC : ActiveNPCActors)
    {
        if (IsValid(NPC))
        {
            NPC->Destroy();
        }
    }
    ActiveNPCActors.Empty();
    RegisteredNPCs.Empty();
    
    Super::Deinitialize();
}

void UQuest_NPCManager::RegisterNPC(const FQuest_NPCProfile& NPCProfile)
{
    RegisteredNPCs.Add(NPCProfile);
    UE_LOG(LogTemp, Warning, TEXT("Registered NPC: %s"), *NPCProfile.NPCName);
}

void UQuest_NPCManager::SpawnNPC(const FString& NPCName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn NPC - no world context"));
        return;
    }

    // Find the NPC profile
    FQuest_NPCProfile* FoundProfile = RegisteredNPCs.FindByPredicate([&NPCName](const FQuest_NPCProfile& Profile)
    {
        return Profile.NPCName == NPCName;
    });

    if (!FoundProfile)
    {
        UE_LOG(LogTemp, Error, TEXT("NPC profile not found: %s"), *NPCName);
        return;
    }

    // Spawn the NPC actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(*FString::Printf(TEXT("QuestNPC_%s"), *NPCName));
    
    AQuest_NPCActor* NewNPC = World->SpawnActor<AQuest_NPCActor>(AQuest_NPCActor::StaticClass(), 
        FoundProfile->SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    
    if (NewNPC)
    {
        NewNPC->NPCProfile = *FoundProfile;
        ActiveNPCActors.Add(NewNPC);
        
        UE_LOG(LogTemp, Warning, TEXT("Spawned NPC: %s at location %s"), 
            *NPCName, *FoundProfile->SpawnLocation.ToString());
    }
}

void UQuest_NPCManager::SpawnAllNPCs()
{
    for (const FQuest_NPCProfile& Profile : RegisteredNPCs)
    {
        if (Profile.bIsActive)
        {
            SpawnNPC(Profile.NPCName);
        }
    }
}

AQuest_NPCActor* UQuest_NPCManager::FindNPCByName(const FString& NPCName)
{
    for (AQuest_NPCActor* NPC : ActiveNPCActors)
    {
        if (IsValid(NPC) && NPC->NPCProfile.NPCName == NPCName)
        {
            return NPC;
        }
    }
    return nullptr;
}

TArray<AQuest_NPCActor*> UQuest_NPCManager::GetNPCsByRole(ENPCRole Role)
{
    TArray<AQuest_NPCActor*> FilteredNPCs;
    
    for (AQuest_NPCActor* NPC : ActiveNPCActors)
    {
        if (IsValid(NPC) && NPC->NPCProfile.Role == Role)
        {
            FilteredNPCs.Add(NPC);
        }
    }
    
    return FilteredNPCs;
}

void UQuest_NPCManager::SetupDefaultNPCs()
{
    // Create Elder quest giver
    FQuest_NPCProfile ElderProfile;
    ElderProfile.NPCName = TEXT("Tribal Elder");
    ElderProfile.Role = ENPCRole::QuestGiver;
    ElderProfile.SpawnLocation = FVector(500.0f, 0.0f, 100.0f);
    
    FQuest_NPCDialogue ElderDialogue1;
    ElderDialogue1.DialogueText = TEXT("The ancient stones whisper of a lost tribe. Follow the river north to find their hunting grounds.");
    ElderDialogue1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781486717394_QuestGiver_Elder.mp3");
    ElderDialogue1.Duration = 14.0f;
    ElderDialogue1.bIsQuestDialogue = true;
    
    ElderProfile.AvailableDialogues.Add(ElderDialogue1);
    ElderProfile.QuestIDs.Add(TEXT("QUEST_LOST_TRIBE"));
    
    RegisterNPC(ElderProfile);
    
    // Create Shaman quest giver
    FQuest_NPCProfile ShamanProfile;
    ShamanProfile.NPCName = TEXT("Tribal Shaman");
    ShamanProfile.Role = ENPCRole::QuestGiver;
    ShamanProfile.SpawnLocation = FVector(-500.0f, 300.0f, 100.0f);
    
    FQuest_NPCDialogue ShamanDialogue1;
    ShamanDialogue1.DialogueText = TEXT("You have proven yourself worthy, survivor. The great hunt begins now.");
    ShamanDialogue1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781486723769_QuestGiver_Shaman.mp3");
    ShamanDialogue1.Duration = 13.0f;
    ShamanDialogue1.bIsQuestDialogue = true;
    
    ShamanProfile.AvailableDialogues.Add(ShamanDialogue1);
    ShamanProfile.QuestIDs.Add(TEXT("QUEST_GREAT_HUNT"));
    
    RegisterNPC(ShamanProfile);
    
    UE_LOG(LogTemp, Warning, TEXT("Default NPCs registered: %d total"), RegisteredNPCs.Num());
}