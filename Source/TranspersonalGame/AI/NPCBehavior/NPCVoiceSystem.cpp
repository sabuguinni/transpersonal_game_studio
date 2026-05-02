#include "NPCVoiceSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"

UNPC_VoiceSystem::UNPC_VoiceSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    VoiceType = ENPC_VoiceType::TribalSurvivor;
    VoiceRange = 1500.0f;
    MinTimeBetweenLines = 3.0f;
    LastVoiceTime = 0.0f;
    bIsSpeaking = false;
}

void UNPC_VoiceSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultVoiceLines();
}

void UNPC_VoiceSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update speaking state
    if (bIsSpeaking)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastVoiceTime > 10.0f) // Max line duration
        {
            bIsSpeaking = false;
        }
    }
}

void UNPC_VoiceSystem::PlayVoiceLine(ENPC_VoiceContext Context)
{
    if (!CanSpeak() || !IsPlayerInRange())
    {
        return;
    }

    FNPC_VoiceLine* VoiceLine = GetBestVoiceLineForContext(Context);
    if (VoiceLine)
    {
        PlaySpecificLine(VoiceLine->Text);
        
        if (!VoiceLine->AudioURL.IsEmpty())
        {
            PlayAudioFromURL(VoiceLine->AudioURL);
        }

        LastVoiceTime = GetWorld()->GetTimeSeconds();
        bIsSpeaking = true;

        UE_LOG(LogTemp, Log, TEXT("NPC Voice: %s"), *VoiceLine->Text);
    }
}

void UNPC_VoiceSystem::PlaySpecificLine(const FString& Text)
{
    if (!CanSpeak())
    {
        return;
    }

    // Display text above NPC (for debugging/subtitles)
    if (GEngine && GetOwner())
    {
        FVector ActorLocation = GetOwner()->GetActorLocation();
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, 
            FString::Printf(TEXT("%s: %s"), *GetOwner()->GetName(), *Text));
    }

    LastVoiceTime = GetWorld()->GetTimeSeconds();
    bIsSpeaking = true;
}

bool UNPC_VoiceSystem::CanSpeak() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return !bIsSpeaking && (CurrentTime - LastVoiceTime) >= MinTimeBetweenLines;
}

void UNPC_VoiceSystem::AddVoiceLine(const FString& Text, const FString& AudioURL, ENPC_VoiceContext Context, float Priority)
{
    FNPC_VoiceLine NewLine;
    NewLine.Text = Text;
    NewLine.AudioURL = AudioURL;
    NewLine.Context = Context;
    NewLine.Priority = Priority;
    
    VoiceLines.Add(NewLine);
}

void UNPC_VoiceSystem::InitializeDefaultVoiceLines()
{
    VoiceLines.Empty();

    // Add pre-generated voice lines with audio URLs
    switch (VoiceType)
    {
        case ENPC_VoiceType::TribalLeader:
            AddVoiceLine("Stay back! The pack is hunting. We must find shelter before nightfall.",
                        "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777707701530_TribalLeader.mp3",
                        ENPC_VoiceContext::Warning, 2.0f);
            AddVoiceLine("Welcome, stranger. Our fire is warm, but beware the shadows.",
                        "", ENPC_VoiceContext::Greeting, 1.0f);
            AddVoiceLine("The great beasts stir. We must move the tribe to higher ground.",
                        "", ENPC_VoiceContext::Warning, 2.5f);
            break;

        case ENPC_VoiceType::TribalSurvivor:
            AddVoiceLine("Fire... fire keeps them away. Gather wood, quickly!",
                        "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777707705236_TribalSurvivor.mp3",
                        ENPC_VoiceContext::Fear, 2.0f);
            AddVoiceLine("I saw tracks by the river. Big ones. Very big.",
                        "", ENPC_VoiceContext::Warning, 1.5f);
            AddVoiceLine("The berries here are safe. My grandmother taught me which ones.",
                        "", ENPC_VoiceContext::Calm, 1.0f);
            break;

        case ENPC_VoiceType::TribalElder:
            AddVoiceLine("In my time, we learned to read the signs. The earth speaks to those who listen.",
                        "", ENPC_VoiceContext::Storytelling, 1.0f);
            AddVoiceLine("These old bones have seen many seasons. Trust in the old ways.",
                        "", ENPC_VoiceContext::Calm, 1.0f);
            break;

        case ENPC_VoiceType::Hunter:
            AddVoiceLine("The wind carries their scent. We are not alone here.",
                        "", ENPC_VoiceContext::Warning, 2.0f);
            AddVoiceLine("My spear is sharp, but against the great lizards... we need more than courage.",
                        "", ENPC_VoiceContext::Combat, 1.5f);
            break;

        default:
            AddVoiceLine("...", "", ENPC_VoiceContext::Calm, 1.0f);
            break;
    }
}

void UNPC_VoiceSystem::PlayAudioFromURL(const FString& AudioURL)
{
    // Note: In a full implementation, this would download and play the audio
    // For now, we'll log the URL for debugging
    UE_LOG(LogTemp, Log, TEXT("Playing audio from URL: %s"), *AudioURL);
    
    // TODO: Implement actual audio playback from URL
    // This would require downloading the MP3 and creating a USoundWave at runtime
}

FNPC_VoiceLine* UNPC_VoiceSystem::GetBestVoiceLineForContext(ENPC_VoiceContext Context)
{
    TArray<FNPC_VoiceLine*> MatchingLines;
    
    // Find all lines matching the context
    for (FNPC_VoiceLine& Line : VoiceLines)
    {
        if (Line.Context == Context)
        {
            MatchingLines.Add(&Line);
        }
    }

    if (MatchingLines.Num() == 0)
    {
        // Fallback to any calm line
        for (FNPC_VoiceLine& Line : VoiceLines)
        {
            if (Line.Context == ENPC_VoiceContext::Calm)
            {
                MatchingLines.Add(&Line);
            }
        }
    }

    if (MatchingLines.Num() == 0)
    {
        return nullptr;
    }

    // Select highest priority line, or random if tied
    FNPC_VoiceLine* BestLine = MatchingLines[0];
    for (FNPC_VoiceLine* Line : MatchingLines)
    {
        if (Line->Priority > BestLine->Priority)
        {
            BestLine = Line;
        }
    }

    return BestLine;
}

bool UNPC_VoiceSystem::IsPlayerInRange() const
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn || !GetOwner())
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    return Distance <= VoiceRange;
}